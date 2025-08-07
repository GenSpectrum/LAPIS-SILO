## Phylogenetic Tree

The phylogenetic tree was created from the `input_file.ndjson` entries. First a new metadata field was created called `usherTree` that was a copy of the `gisaid_epi_isl` field.

```bash
jq -c '.metadata.usherTree = .metadata.gisaid_epi_isl' input_file.ndjson > output.ndjson
```

Then the sequences in the ndjson file were extracted into a fasta file where the `usherTree` column was used as the fasta entry header. Then a tree was generated using [`augur tree`](https://github.com/nextstrain/augur), additionally to label the internal nodes `augur refine` was run.

```bash
jq -r '
  # for each JSON object (.),
  # grab the ID and sequence, then format as FASTA
  . as $obj
  | ">" + ($obj.metadata.usherTree) 
    + "\n" 
    + ($obj.alignedNucleotideSequences.main)
' input_file.ndjson > output.fasta

augur tree --alignment output.fasta --output initial.tree

augur refine  --tree initial.tree --alignment output.fasta --output-tree output.tree
```

Later `EPI_ISL_1001493` and `EPI_ISL_1747752` were removed from the tree and the node `EPI_ISL_1408408` was set to null in the metadata. This ensures that the missing node count should always be 3.