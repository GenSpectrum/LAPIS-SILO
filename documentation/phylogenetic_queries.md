# Documentation: Phylogenetic Queries

By default SILO does not support phylogeny-based queries. In order to enable these queries a file containing the phylogeny of all (or a subset) of sequences as leaf nodes needs to be imported during pre-processing. The tree file can be specified with the `lineageDefinitionsFilename` field of the preprocessing config. Additionally, to enable querying users must specify which metadata column corresponds to the nodes in the tree file. This should be done by adding the `phyloTreeNodeIdentifier` to the respective metadata field in the database config.

> **Warning**
>
> Like the `generateLineageIndex` option the `phyloTreeNodeIdentifier` option can only be applied to metadata fields that are strings. However, unlike the `generateLineageIndex` option the `generateIndex` should NOT be set. (Internally lineage indexes are queried using a bitmap structure, however for phylogenetic queries we use the tree structure directly).

## Tree File Format

We allow users to specify a tree phylogeny using two standard formats: [newick](https://en.wikipedia.org/wiki/Newick_format) and [auspice.json (v2)](https://docs.nextstrain.org/projects/auspice/en/stable/releases/v2.html#new-dataset-json-format). We do not currently support ancestral reassortment graphs. Unlike the standard format we additionally require that **all nodes (internal and leaves) of the tree should be uniquely labelled**.

## Phylogenetic Filters

As described in [Query Documentation](query_documentation.md) queries consist of a filter (can be thought of as an sql `WHERE` clause) and an action (specifies the return type of a query and if additional operations should be performed on the filtered data).

### PhyloDescendantOf

Filters sequences to the subset of all sequences that are a child of an internal node in the tree. `COLUMN_NAME` must correspond to the name of a STRING column with the `phyloTreeNodeIdentifier` and a corresponding tree. `INTERNAL_NODE_NAME` must correspond to a node that exists in the linked tree.

```json
"filterExpression": {
      "type": "PhyloDescendantOf",
      "column": "COLUMN_NAME",
      "internalNode": "INTERNAL_NODE_NAME"
    }
```

## Phylogenetic Actions

### MRCA (Most recent common ancestor)

```json
"action": {
      "type": "MostRecentCommonAncestor",
      "columnName": "COLUMN_NAME",
      "printNodesNotInTree": true
    }
```

Returns the most recent common ancestor of all sequences in the filter it is applied to. If sequences included in the filter do not exist in the phylogenetic tree they are ignored and the count of such missing sequences is added as a field `missingNodeCount`. Additionally, if desired, a list of all missing nodes can be returned as a comma-separated list by setting `printNodesNotInTree` to true (default is false). Note in the query shown above `COLUMN_NAME` must correspond to the name of a STRING column with the `phyloTreeNodeIdentifier` and a corresponding tree.

The result of such a query is a ndjson with a single row, where `missingFromTree` is only added if `printNodesNotInTree` is set to true:

```json
{
  "mrcaNode": "MRCA_NODE",
  "missingNodeCount": "INT",
  "missingFromTree": "MISSING_NODE1,MISSING_NODE2"
}
```
