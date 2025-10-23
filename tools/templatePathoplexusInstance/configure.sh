#!/usr/bin/env bash
set -euo pipefail

ORGANISM="$1"

OUTDIR="../../testBaseData/ppxData/${ORGANISM}"
mkdir -p "$OUTDIR"

# Get data from Loculus backend
echo "Fetching data for ${ORGANISM} ..."
curl -X 'GET' \
  "https://backend.pathoplexus.org/${ORGANISM}/get-released-data?compression=ZSTD" \
  -H 'accept: application/x-ndjson' -o "${OUTDIR}/get-released-data.ndjson.zst"

# Run legacyNdjsonTransformer on data
echo "Transforming data for ${ORGANISM} ..."
zstdcat "${OUTDIR}/get-released-data.ndjson.zst" | \
    "../legacyNdjsonTransformer/target/release/legacy-ndjson-transformer" | \
    zstd > "${OUTDIR}/get-released-data.transformed.ndjson.zst"

# Get config files from LAPIS
echo "Fetching config files for ${ORGANISM} ..."
echo "Fetching database config ..."
curl -X 'GET' \
  "https://lapis.pathoplexus.org/${ORGANISM}/sample/databaseConfig?downloadAsFile=false" \
  -H 'accept: application/yaml' -o "${OUTDIR}/database_config.yaml"
echo "Fetching reference genome ..."
curl -X 'GET' \
  "https://lapis.pathoplexus.org/${ORGANISM}/sample/referenceGenome?downloadAsFile=false" \
  -H 'accept: application/json' -o "${OUTDIR}/reference_genomes.json"

echo "Parse database config to see if lineage definitions are required ..."
names=$(yq '.schema.metadata[] | select(.generateLineageIndex == true or .generateLineageIndex == "true") | .name' "${OUTDIR}/database_config.yaml")
for name in $names; do
  echo "Fetching lineage definition for $name ..."
  curl -s -X 'GET' \
    "https://lapis.pathoplexus.org/${ORGANISM}/sample/lineageDefinition/${name}?downloadAsFile=false" \
    -H 'accept: application/json' \
    -o "${OUTDIR}/lineage_definition_${name}.yaml"
done

# For v.0.9.0 and later, we need to refactor the database config to the new lineage definition format
echo "Refactor database config to new lineage definition format ..."
yq '
  # change true -> "lineage_definition_<name>"
  (.schema.metadata[] | select(.generateLineageIndex == true or .generateLineageIndex == "true"))
    |= (.generateLineageIndex = "lineage_definition_" + .name)
  |
  del(.schema.metadata[] | select(.generateLineageIndex == false or .generateLineageIndex == "false") | .generateLineageIndex)
' "${OUTDIR}/database_config.yaml" > "${OUTDIR}/database_config.modified.yaml"

mv "${OUTDIR}/database_config.yaml" "${OUTDIR}/database_config.old.yaml"
mv "${OUTDIR}/database_config.modified.yaml" "${OUTDIR}/database_config.yaml"

echo "Creating preprocessing config ..."
config_file="${OUTDIR}/preprocessing_config.yaml"

{
  echo "ndjsonInputFilename: \"get-released-data.transformed.ndjson.zst\""
  if [[ -n "$names" ]]; then
    echo "lineageDefinitionFilenames:"
    for name in $names; do
      echo "  - \"lineage_definition_${name}.yaml\""
    done
  fi
  echo "referenceGenomeFilename: \"reference_genomes.json\""
} > "$config_file"

echo "Created ${config_file}"

echo "Copying docker_compose_template.yml to ${OUTDIR}/docker_compose.yml ..."
cp docker_compose_template.yml "${OUTDIR}/docker_compose.yml"

echo "Done setting up ${OUTDIR}."