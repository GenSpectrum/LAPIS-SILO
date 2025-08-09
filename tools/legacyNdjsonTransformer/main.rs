use serde::{Deserialize};
use serde_json::{Map, Value};
use std::io::{self, BufRead, BufReader};

#[derive(Debug, Deserialize)]
struct InputData {
    metadata: Map<String, Value>,
    #[serde(rename = "alignedNucleotideSequences")]
    aligned_nucleotide_sequences: Map<String, Value>,
    #[serde(rename = "unalignedNucleotideSequences")]
    unaligned_nucleotide_sequences: Map<String, Value>,
    #[serde(rename = "alignedAminoAcidSequences")]
    aligned_amino_acid_sequences: Map<String, Value>,
    #[serde(rename = "nucleotideInsertions")]
    nucleotide_insertions: Map<String, Value>,
    #[serde(rename = "aminoAcidInsertions")]
    amino_acid_insertions: Map<String, Value>,
}

fn transform_data(input: InputData) -> Map<String, Value> {
    let mut result = Map::new();

    // Add metadata fields directly to the result
    for (key, value) in input.metadata {
        result.insert(key, value);
    }

    // Process aligned nucleotide sequences
    for (segment_key, sequence_value) in input.aligned_nucleotide_sequences {
        if sequence_value.is_null() {
            result.insert(segment_key.clone(), Value::Null);
        } else {
            let insertions = input.nucleotide_insertions
                .get(&segment_key)
                .cloned()
                .unwrap_or(Value::Array(vec![]));

            let mut seq_obj = Map::new();
            seq_obj.insert("sequence".to_string(), sequence_value);
            seq_obj.insert("insertions".to_string(), insertions);

            result.insert(segment_key, Value::Object(seq_obj));
        }
    }

    // Process aligned amino acid sequences
    for (gene_key, sequence_value) in input.aligned_amino_acid_sequences {
        if sequence_value.is_null() {
            result.insert(gene_key.clone(), Value::Null);
        } else {
            let insertions = input.amino_acid_insertions
                .get(&gene_key)
                .cloned()
                .unwrap_or(Value::Array(vec![]));

            let mut seq_obj = Map::new();
            seq_obj.insert("sequence".to_string(), sequence_value);
            seq_obj.insert("insertions".to_string(), insertions);

            result.insert(gene_key, Value::Object(seq_obj));
        }
    }

    // Process unaligned nucleotide sequences
    for (segment_key, sequence_value) in input.unaligned_nucleotide_sequences {
        let unaligned_key = format!("unaligned_{}", segment_key);
        result.insert(unaligned_key, sequence_value);
    }

    result
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let stdin = io::stdin();
    let reader = BufReader::new(stdin.lock());

    for line in reader.lines() {
        let line = line?;
        let line = line.trim();

        if line.is_empty() {
            continue;
        }

        let input_data: InputData = serde_json::from_str(line)?;

        let transformed = transform_data(input_data);

        let output = serde_json::to_string(&transformed)?;
        println!("{}", output);
    }

    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use serde_json::json;

    #[test]
    fn test_transform_null_values() {
        let input = r#"{
            "metadata": {"key": "id1", "col": "A"},
            "alignedNucleotideSequences": {"segment1": null},
            "unalignedNucleotideSequences": {"segment1": null},
            "alignedAminoAcidSequences": {"gene1": null},
            "nucleotideInsertions": {"segment1": []},
            "aminoAcidInsertions": {"gene1": []}
        }"#;

        let input_data: InputData = serde_json::from_str(input).unwrap();
        let result = transform_data(input_data);

        assert_eq!(result.get("key"), Some(&json!("id1")));
        assert_eq!(result.get("col"), Some(&json!("A")));
        assert_eq!(result.get("segment1"), Some(&json!(null)));
        assert_eq!(result.get("gene1"), Some(&json!(null)));
        assert_eq!(result.get("unaligned_segment1"), Some(&json!(null)));
    }

    #[test]
    fn test_transform_with_data() {
        let input = r#"{
            "metadata": {"key": "id2", "col": "B"},
            "alignedNucleotideSequences": {"segment1": "A"},
            "unalignedNucleotideSequences": {"segment1": "C"},
            "alignedAminoAcidSequences": {"gene1": "Y"},
            "nucleotideInsertions": {"segment1": ["123","456"]},
            "aminoAcidInsertions": {"gene1": ["1","2"]}
        }"#;

        let input_data: InputData = serde_json::from_str(input).unwrap();
        let result = transform_data(input_data);

        assert_eq!(result.get("key"), Some(&json!("id2")));
        assert_eq!(result.get("col"), Some(&json!("B")));
        assert_eq!(result.get("unaligned_segment1"), Some(&json!("C")));

        let segment1 = result.get("segment1").unwrap();
        assert_eq!(segment1.get("sequence"), Some(&json!("A")));
        assert_eq!(segment1.get("insertions"), Some(&json!(["123", "456"])));

        let gene1 = result.get("gene1").unwrap();
        assert_eq!(gene1.get("sequence"), Some(&json!("Y")));
        assert_eq!(gene1.get("insertions"), Some(&json!(["1", "2"])));
    }
}
