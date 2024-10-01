import polars as pl
import json
import csv
import argparse
import duckdb
import shutil
import sys
import yaml
import os
import tempfile

parser = argparse.ArgumentParser(description="Process a JSON file and convert it to a DataFrame.")
parser.add_argument('alias_key', type=str, help='Path to the alias_key in JSON format')
parser.add_argument('lineage_file', type=str, help='Path to the input_file containing all lineages')
parser.add_argument('--preserve-tmp-dir', action='store_true', help='Preserve the temporary directory to keep the intermediate duckdb tables')
parser.add_argument('--verbose', '-v', action='store_true', help='Verbose logging')

args = parser.parse_args()

alias_table = []
alias_dict = dict()

with open(args.alias_key, 'r') as file:
    alias_file = json.load(file)

    for key, value in alias_file.items():
        if isinstance(value, str):
            if value and len(value) > 0:
                alias_table.append({"name": key, "alias": value})
                alias_dict[key] = [value]
            else:
                alias_table.append({"name": key, "alias": key})
                alias_dict[key] = [key]
        else:
            alias_dict[key] = value

def unalias_lineage(lineage):
    parts = lineage.split('.')
    part_before_first_dot = parts[0]
    if part_before_first_dot in alias_dict and len(alias_dict[part_before_first_dot]) == 1:
        parts[0] = alias_dict[part_before_first_dot][0]
        return '.'.join(parts)
    else:
        return lineage

def find_immediate_parent(lineage):
    if '.' in lineage:
        return lineage.rsplit('.', 1)[0]
    return None

lineage_table = []

all_unaliased = set()

with open(args.lineage_file, mode='r', newline='', encoding='utf-8') as infile:
    reader = csv.reader(infile)
    for row in reader:
        lineage = row[0]
        unaliased = unalias_lineage(lineage)
        all_unaliased.add(unaliased)
        parent_lineage_unaliased = find_immediate_parent(unaliased)
        lineage_table.append({"lineage": lineage, "unaliased": unaliased, "parent_lineage_unaliased": parent_lineage_unaliased})

idx = 0
while idx < len(lineage_table):
    unaliased = lineage_table[idx]["parent_lineage_unaliased"]
    if unaliased and unaliased not in all_unaliased:
        all_unaliased.add(unaliased)
        parent_lineage_unaliased = find_immediate_parent(unaliased)
        lineage_table.append({"lineage": unaliased, "unaliased": unaliased, "parent_lineage_unaliased": parent_lineage_unaliased})
    idx += 1

lineage_table_df = pl.DataFrame(lineage_table)
alias_table_df = pl.DataFrame(alias_table)

temp_dir = tempfile.mkdtemp()
if args.verbose:
    print(f"Temporary directory: {temp_dir}", file=sys.stderr)
db_path = os.path.join(temp_dir, "lineage_transform.duckdb")
con = duckdb.connect(db_path)
con.execute(f"CREATE TABLE alias_key (alias_name VARCHAR, alias_value VARCHAR)")
con.execute(f"INSERT INTO alias_key SELECT * FROM alias_table_df")

con.execute(f"CREATE TABLE lineages (lineage VARCHAR, unaliased VARCHAR, parent_lineage_unaliased VARCHAR)")
con.execute(f"INSERT INTO lineages SELECT * FROM lineage_table_df")

con.execute(f"""
            CREATE TABLE parent_child_relations AS (
            SELECT DISTINCT child.lineage AS lineage, parent.lineage AS parent
            FROM lineages child, lineages parent
            WHERE child.parent_lineage_unaliased = parent.unaliased)
""")

con.execute(f"""
            CREATE TABLE all_aliases AS (
            SELECT lineage, alias_name || substr(unaliased, strlen(alias_value) + 1, strlen(unaliased) - strlen(alias_value)) as valid_alias
            FROM lineages, alias_key
            WHERE (unaliased = alias_value OR unaliased LIKE alias_value || '.%') 
            AND lineage <> valid_alias
            )
""")

query = "SELECT lineage FROM lineages ORDER BY lineage"
data = con.execute(query).fetchall()

lineage_dict = {}
for lineage, in data:
    lineage_dict[lineage] = {}

query = "SELECT lineage, parent FROM parent_child_relations ORDER BY lineage"
data = con.execute(query).fetchall()

for lineage, parent in data:
    if "parents" not in lineage_dict[lineage]:
        lineage_dict[lineage]["parents"] = []
    lineage_dict[lineage]["parents"].append(parent)

query = "SELECT lineage, valid_alias FROM all_aliases ORDER BY lineage"
data = con.execute(query).fetchall()

for lineage, valid_alias in data:
    if "aliases" not in lineage_dict[lineage]:
        lineage_dict[lineage]["aliases"] = []
    lineage_dict[lineage]["aliases"].append(valid_alias)

yaml.dump(lineage_dict, sys.stdout, default_flow_style=False)

if args.preserve_tmp_dir:
    print(f"Temporary directory containing the backing duckdb file: {temp_dir}", file=sys.stderr)
else:
    shutil.rmtree(temp_dir)
