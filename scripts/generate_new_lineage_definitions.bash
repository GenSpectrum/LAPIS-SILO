#!/bin/bash 
curl https://raw.githubusercontent.com/cov-lineages/pango-designation/refs/heads/master/pango_designation/alias_key.json > alias_key.json 
curl https://raw.githubusercontent.com/cov-lineages/pango-designation/refs/heads/master/lineages.csv | tail -n +2 | cut -d',' -f2 > lineages.csv
python3 alias2lineageDefinitions.py alias_key.json lineages.csv > lineage_definitions.yaml
