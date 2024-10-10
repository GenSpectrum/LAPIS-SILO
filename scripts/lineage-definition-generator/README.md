# Generate Lineage Definitions

## Overview

**Generate Lineage Definitions** is a TypeScript-based utility for processing JSON alias keys and CSV lineage files to transform them into a structured format using DuckDB. It reads an alias key in JSON format, a lineage file in CSV format, and outputs a structured YAML file, representing lineages and their relationships (parents and aliases).

## Installation

### Prerequisites

Make sure you have **Node.js** installed.

### Steps

1. Clone the repository:
    ```bash
    git clone https://github.com/GenSpectrum/LAPIS-SILO
    ```

2. Navigate to the project directory:
    ```bash
    cd scripts/lineage-definition-generator
    ```

3. Install the required dependencies:
    ```bash
    npm install
    ```

## Usage

To run the transformation script, use the following command:

```bash
npm run start -- <aliasKey> <lineageFile> [options]
```

### Arguments 

* `<aliasKey>`: The path to the JSON file containing alias key mappings.
* `<lineageFile>`: The path to the CSV file containing all lineages. 

### Options 

* `--preserve-tmp-dir`: Preserve the temporary directory where intermediate DuckDB files are stored. By default, the directory is deleted after execution. 
* `--verbose, -v`: Enable verbose logging for more detailed output during the execution.

Output
The resulting lineage structure is printed in YAML format:

### Output 

The resulting lineage structure is printed in YAML format: 


```
parent1:
  aliases:
  - some_alias
parent2: {}
lineage1: 
  parents: 
  - parent1 
  - parent2 
  aliases: 
  - alias1```

