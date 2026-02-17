# SILO HTTP API Reference

SILO includes an HTTP API for querying preprocessed outputs.

## Running the API Server

```bash
silo api \
  --database-config database_config.yaml \
  --runtime-config runtime_config.yaml
```

### Runtime Configuration

Configuration is resolved in order of precedence: CLI arguments > environment variables (prefixed `SILO_`) > config file > defaults.

| Key | Default | Description |
|-----|---------|-------------|
| `dataDirectory` | `output` | Path to preprocessed database |
| `api.port` | `8081` | HTTP listen port |
| `api.maxQueuedHttpConnections` | `256` | Maximum queued connections |
| `api.threadsForHttpConnections` | `0` | Worker threads (0 = number of CPUs) |
| `api.estimatedStartupTimeInMinutes` | — | Used in `Retry-After` header during startup |
| `query.materializationCutoff` | `32767` | Batch size threshold for streaming. (Note: batch size of results is not guaranteed to stay below this number) |

## Common Response Headers

Every response includes:

| Header | Description |
|--------|-------------|
| `X-Request-Id` | Echoes the request's `X-Request-Id`, or a generated UUID v4 if none was provided. Useful for correlating logs. |
| `data-version` | A 10-digit Unix timestamp identifying the database snapshot used to serve the request. Allows clients to detect when the underlying data has changed between requests. |

## Endpoints

### `GET /health`

Returns the server health status.

**Response** (200, `application/json`):
```json
{"status": "UP"}
```

During startup, before the database is loaded, the server returns 503 with a `Retry-After` header.

---

### `GET /info`

Returns metadata about the loaded database.

**Response** (200, `application/json`):
```json
{
  "version": "0.1.0",
  "sequenceCount": 100,
  "horizontalBitmapsSize": 5594,
  "verticalBitmapsSize": 28102,
  "numberOfPartitions": 1
}
```

| Field | Description |
|-------|-------------|
| `version` | SILO software version |
| `sequenceCount` | Total number of sequences in the database |
| `horizontalBitmapsSize` | Size of horizontal bitmap indexes (bytes) |
| `verticalBitmapsSize` | Size of vertical bitmap indexes (bytes) |
| `numberOfPartitions` | Number of table partitions |

---

### `GET /lineageDefinition/{columnName}`

Returns the lineage tree definition for the given column.

**Parameters:**
- `columnName` (path) — Name of a column with `generateLineageIndex: true`

**Response** (200, `application/yaml`): the lineage definition YAML file.

**Errors** (400):
- Column does not exist
- Column is not of type `indexed-string`
- Column does not have a lineage tree defined

---

### `POST /query`

Executes a query against the database. See [query_documentation.md](query_documentation.md) for the full filter and action reference.

**Request** (`application/json`):
```json
{
  "filterExpression": { "type": "True" },
  "action": { "type": "Aggregated" }
}
```

Both `filterExpression` and `action` are required.

**Errors** (400, `application/json`):
```json
{
  "error": "Bad request",
  "message": "description of what went wrong"
}
```

Queries time out after 120 seconds.

#### Output Format Negotiation

The output format is selected via the HTTP `Accept` header:

| Accept header value | Content-Type | Format |
|---------------------|--------------|--------|
| *(default, or any other value)* | `application/x-ndjson` | Newline-delimited JSON |
| `application/vnd.apache.arrow.stream` | `application/vnd.apache.arrow.stream` | Apache Arrow IPC stream |

##### NDJSON (default)

Each result row is a self-contained JSON object on its own line:

```
{"count":100}
```

NDJSON is human-readable and easy to consume from any language. However, because each line is independent, there is no built-in way for a client to distinguish a complete response from one that was truncated mid-stream (e.g. due to a network interruption or server timeout). A client that reads partial NDJSON will silently receive fewer rows than expected.

##### Arrow IPC Streaming

Request this format by setting the `Accept` header:

```
Accept: application/vnd.apache.arrow.stream
```

The response is a binary [Apache Arrow IPC stream](https://arrow.apache.org/docs/format/Columnar.html#ipc-streaming-format). This format:

- **Guards against incomplete downloads.** The stream begins with a schema message and ends with an explicit end-of-stream marker written by `Close()`. Arrow client libraries will raise an error when reading a stream that is missing this marker, so a truncated response (from a network drop, timeout, or server crash) is detected automatically rather than silently returning partial results.
- **Preserves type information.** Column types (integers, floats, booleans, strings) are carried in the schema rather than inferred from text, avoiding issues like integer-vs-float ambiguity in JSON.
- **Is efficient for large result sets.** Data is columnar and binary-encoded, avoiding the overhead of JSON serialization and text parsing.

Arrow IPC libraries are available for Python (`pyarrow`), R (`arrow`), JavaScript (`apache-arrow`), Rust, Go, Java, and many other languages.

Example with `curl` and Python:

```bash
curl -X POST \
  -H "Accept: application/vnd.apache.arrow.stream" \
  -d '{"action":{"type":"Aggregated"},"filterExpression":{"type":"True"}}' \
  http://localhost:8081/query --output result.arrow
```

```python
import pyarrow.ipc as ipc

with open("result.arrow", "rb") as f:
    reader = ipc.open_stream(f)
    table = reader.read_all()

print(table.to_pandas())
```

---

## Error Responses

All error responses are JSON, regardless of the `Accept` header.

| Status | Meaning |
|--------|---------|
| 400 | Bad request — malformed JSON, invalid query, unknown filter/action type |
| 404 | Unknown endpoint |
| 405 | HTTP method not allowed on this endpoint |
| 500 | Internal server error |
| 503 | Database not yet loaded; `Retry-After` header indicates estimated wait |

Error body:
```json
{
  "error": "Bad request",
  "message": "detailed description"
}
```
