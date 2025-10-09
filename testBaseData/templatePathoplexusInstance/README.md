# Template Pathoplexus Instance

This is a config for reproducing the current Pathoplexus LAPIS-SILO instances locally or in a docker container using the latest LAPIS/SILO version. The script requires passing the Pathoplexus organism as an argument, it will create a folder <ORGANISM> with the config. Run the script with:

```bash
bash configure.sh <ORGANISM>
```

To run SILO locally using this configuration:

```bash
cd <ORGANISM>
../../../build/Release/silo preprocessing --database-config database_config.yaml --preprocessing-config preprocessing_config.yaml
../../../build/Release/silo api --api-port 8093
```

To run SILO and LAPIS in a docker container you can run:

```bash
SILO_PORT=8093 LAPIS_PORT=8092 docker compose -f docker-compose.yml up
```
To test the instance you can send LAPIS queries in the form of:

```
curl http://localhost:8092/sample/unalignedNucleotideSequences
```

and SILO queries in the form of:

```
curl -X POST http://localhost:8093/query \
  -H "Content-Type: application/json" \
  -d '{
      "action": {
        "type": "Fasta",
        "sequenceNames": ["unaligned_main"]
      },
      "filterExpression": {
        "type": "True"
      }
  }'
```

To test thread-related issues it makes sense to limit the number of requests that can be handled by SILO using `--api-max-queued-http-connections` when running the SILO api.

## Earlier SILO/LAPIS versions

To use a version of SILO older than v0.9.0 use the `database_config.old.yaml` instead of the `database_config.yaml`, for versions older than v0.8 additionally use `get-released-data.ndjson.zst` instead of `get-released-data.modified.ndjson.zst`.

