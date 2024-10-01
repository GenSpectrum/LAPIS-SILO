#!/bin/bash 
curl https://raw.githubusercontent.com/cov-lineages/pango-designation/refs/heads/master/pango_designation/alias_key.json > alias_key.json 
curl https://raw.githubusercontent.com/cov-lineages/pango-designation/refs/heads/master/lineages.csv > lineages.csv
python3 uniqueValuesFromColumn.py lineages.csv 1 lineage_keys.csv
python3 alias2lineageDefinitions.py alias_key.json lineage_keys.csv > lineage_definitions.yaml
