# Documentation: Monophyletic Search Using Lineage DAGs
This specification defines a simple YAML-based input format to enable general monophyletic searches on a tree of lineages. 
By treating lineage names as string labels in a directed acyclic graph (DAG), this approach avoids grammar complexity while remaining versatile and easy to implement.

## Overview
Purpose: Provide a straightforward format for defining lineage relationships for monophyletic search.
Input Format: A YAML configuration file represents lineage relationships in a DAG structure using parent-child relationships and aliases.

## Input Format
The input format is written in YAML and defines lineage labels in terms of:
- parents: Specifies the direct ancestors of the lineage.
- aliases: A list of alternative names for the lineage.

### YAML Structure
```
<lineage_label>:
  parents: [ <lineage_label of parent> ...]
  aliases: [ <alias_label> ...]
```

- `<lineage_label>`: The name of the lineage (unique identifier).
- `parents`: Optional list of parent lineage labels.
- `aliases`: Optional list of alternative names for the lineage.


## Examples
### Example 1: Basic Lineage Tree
A simple tree with a root (`A`) and two child lineages (`B` and `C`):

```
A:
  aliases:
    - Root

B:
  parents: 
    - A
  aliases:
    - BranchB

C:
  parents: 
    - A
  aliases:
    - BranchC
```

### Example 2: Lineage tree with recombinant
A more complex example where lineage `E` has multiple parents (`B` and `D`):

```
A:
  aliases:
    - Root

B:
  parents: 
    - A
  aliases:
    - BranchB

C:
  parent:
    - A
  aliases:
    - BranchC

D:
  parent: 
    - B
  aliases:
    - SubBranchD

E:
  parent: [B, D]
  aliases:
    - LeafE
```

## Key Considerations
- Alias Flexibility: Use aliases to standardize alternative names for lineages.
- Root Lineages: Specify root lineages with `parents: null`, `parents: []`, or by omitting the key `parents`
- Minimal assumptions: SILO verifies that the lineage labels are unique and the edges contain no cycles. No further assumptions about the lineage system are made