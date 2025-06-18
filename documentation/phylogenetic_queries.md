# Documentation: Phylogenetic Queries

By default SILO does not support phylogeny-based queries. In order to enable these queries a file containing the phylogeny of all (or a subset) of sequences as leaf nodes needs to be imported during pre-processing. The tree file can be specified with the `lineageDefinitionsFilename` field of the preprocessing config. Additionally, to enable querying users must specify which metadata column corresponds to the nodes in the tree file. This should be done by adding the `phyloTreeNodeIdentifier` to the respective metadata field in the database config.

> **Warning**
> 
> Like the `generateLineageIndex` option the `phyloTreeNodeIdentifier` option can only be applied to metadata fields that are strings. However, unlike the `generateLineageIndex` option the `generateIndex` should NOT be set. (Internally lineage indexes are queried using a bitmap structure, however for phylogenetic queries we use the tree structure directly).

## Tree File Format

We allow users to specify a tree phylogeny using two standard formats: [newick](https://en.wikipedia.org/wiki/Newick_format) and [auspice.json (v2)](https://docs.nextstrain.org/projects/auspice/en/stable/releases/v2.html#new-dataset-json-format). We do not currently support ancestral reassortment graphs. Unlike the standard format we additionally require that **all nodes (internal and leaves) of the tree should be uniquely labelled**. 

// TODO: Add details of the phylogenetic queries that SILO now supports

