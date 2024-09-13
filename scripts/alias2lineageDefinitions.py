import polars as pl
import json
import argparse
import duckdb
import yaml
import shutil
import sys
import os
import tempfile


# Define the argument parser
parser = argparse.ArgumentParser(description="Process a JSON file and convert it to a DataFrame.")
parser.add_argument('alias_key', type=str, help='Path to the alias_key in JSON format')
parser.add_argument('lineage_file', type=str, help='Path to the input_file containing all lineages')
parser.add_argument('--preserve-tmp-dir', action='store_true', help='Preserve the temporary directory to keep the intermediate duckdb tables')
parser.add_argument('--verbose', '-v', action='store_true', help='Verbose logging')

# Parse the arguments
args = parser.parse_args()

# Load the JSON data from the file path provided as argument
with open(args.alias_key, 'r') as file:
    alias_key = json.load(file)

# Create a list to store the reformatted data
reformatted_data = []

# Loop through the JSON and format it as desired
for key, value in alias_key.items():
    #if isinstance(value, list):
    #    for list_element in value:
    #        reformatted_data.append({"name": key, "alias": list_element})
    if value and isinstance(value, str):  
        reformatted_data.append({"name": key, "alias": value})
    else:
        reformatted_data.append({"name": key, "alias": key})


# Convert the list of dictionaries to a Polars DataFrame
df = pl.DataFrame(reformatted_data)

temp_dir = tempfile.mkdtemp()
if args.verbose:
    print(f"Temporary directory: {temp_dir}", file=sys.stderr)
db_path = os.path.join(temp_dir, "lineage_transform.duckdb")
con = duckdb.connect(db_path)
con.execute(f"CREATE TABLE alias_key (alias_name VARCHAR, alias_value VARCHAR)")
con.execute(f"INSERT INTO alias_key SELECT * FROM df")

con.execute(f"CREATE TABLE lineages (lineage VARCHAR)")
con.execute(f"INSERT INTO lineages SELECT DISTINCT * FROM '{args.lineage_file}'")

con.execute(f"""
            CREATE TABLE lineages_unaliased AS (
            SELECT 
                lineage, 
                alias_value || 
                CASE 
                    WHEN instr(lineage, '.') > 0 THEN substr(lineage, instr(lineage, '.'))
                    ELSE ''
                END AS unaliased
            FROM lineages
            JOIN alias_key
            ON alias_key.alias_name = 
                CASE 
                    WHEN instr(lineage, '.') > 0 THEN substr(lineage, 1, instr(lineage, '.') - 1)
                    ELSE lineage
                END
            )
""")

con.execute(f"""
            CREATE TABLE all_lineages_unaliased AS
            WITH RECURSIVE filled_gaps AS (
                -- Start with the original values
                SELECT lineage, unaliased
                FROM lineages_unaliased
                
                UNION ALL
                
                -- Recursively remove the last ".XX" from the current value
                SELECT CASE 
                           WHEN instr(unaliased, '.') > 0 THEN substr(unaliased, 1, length(unaliased) - instr(reverse(unaliased), '.'))
                       END AS lineage,
                       CASE 
                           WHEN instr(unaliased, '.') > 0 THEN substr(unaliased, 1, length(unaliased) - instr(reverse(unaliased), '.'))
                       END AS unaliased
                FROM filled_gaps
                WHERE NOT EXISTS ( SELECT * FROM lineages_unaliased WHERE lineages_unaliased.unaliased = filled_gaps.unaliased )
            )
            SELECT DISTINCT lineage, unaliased
            FROM filled_gaps
            WHERE lineage IS NOT NULL
            ORDER BY lineage
""")

con.execute(f"""
            CREATE TABLE with_unaliased_parent_lineage AS (
            SELECT lineage, 
            CASE 
                WHEN instr(REVERSE(unaliased), '.') > 0 THEN REVERSE(SUBSTR(REVERSE(unaliased), instr(REVERSE(unaliased), '.') + 1))
            END AS parent_lineage
            FROM all_lineages_unaliased )
""")

con.execute(f"""
            CREATE TABLE parent_child_relations AS (
            SELECT DISTINCT child.lineage AS lineage, parent.lineage AS parent, parent.unaliased as unaliased_parent
            FROM with_unaliased_parent_lineage child, all_lineages_unaliased parent
            WHERE child.parent_lineage = parent.unaliased)
""")

con.execute(f"""
            CREATE TABLE all_aliases AS (
            SELECT lineage, alias_name || substr(unaliased, strlen(alias_value) + 1, strlen(lineage) - strlen(alias_value))
            FROM all_lineages_unaliased, alias_key
            WHERE unaliased LIKE alias_value || '%'
            )
""")

query = "SELECT lineage FROM all_lineages_unaliased ORDER BY lineage"
data = con.execute(query).fetchall()

lineage_dict = {}
for lineage in data:
    lineage_dict[lineage] = {}
    lineage_dict[lineage]["parents"] = []
    lineage_dict[lineage]["aliases"] = []

query = "SELECT lineage, parent FROM parent_child_relations ORDER BY lineage"
data = con.execute(query).fetchall()

for lineage, parent in data:
    lineage_dict[lineage]["parents"].append(parent)

yaml.dump(lineage_dict, sys.stdout, default_flow_style=False)

if args.preserve_tmp_dir:
    print(f"Temporary directory containing the backing duckdb file: {temp_dir}", file=sys.stderr)
else:
    shutil.rmtree(temp_dir)
