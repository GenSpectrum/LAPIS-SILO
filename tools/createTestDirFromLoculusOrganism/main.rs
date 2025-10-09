use clap::Parser;
use reqwest::Client;
use serde::{Deserialize, Serialize};
use serde_yaml_ng::Value;
use std::collections::HashMap;
use std::fs;
use std::process::Command;
use tokio;

#[derive(Parser)]
struct Args {
    #[arg(short, long, default_value = "pathoplexus.org")]
    loculus_url: String,
    #[arg(short, long)]
    organism_name: String,
    #[arg(short, long, default_value = "false")]
    transform_to_v8: bool,
}

#[derive(Debug, Deserialize)]
struct DatabaseConfig {
    schema: Schema,
}

#[derive(Debug, Deserialize)]
struct Schema {
    #[serde(rename = "instanceName")]
    instance_name: String,
    metadata: Vec<MetadataField>,
    #[serde(rename = "primaryKey")]
    primary_key: String,
}

#[derive(Debug, Deserialize)]
struct MetadataField {
    name: String,
    #[serde(rename = "type")]
    field_type: String,
    #[serde(rename = "isPhyloTreeField", default)]
    is_phylo_tree_field: bool,
    #[serde(rename = "generateIndex", default)]
    generate_index: bool,
    #[serde(rename = "generateLineageIndex", default)]
    generate_lineage_index: bool,
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let args = Args::parse();

    let lapis_organism_url = format!("https://lapis.{}/{}/sample", args.loculus_url, args.organism_name);
    let backend_organism_url = format!("https://backend.{}/{}", args.loculus_url, args.organism_name);


    // Create a reqwest client
    let client = reqwest::Client::new();

    // Download and parse database config
    let database_config = download_database_config(&client, &lapis_organism_url).await?;

    // Parse the YAML config
    let config: DatabaseConfig = serde_yaml_ng::from_str(&database_config)?;

    // Find columns with generateLineageIndex: true
    let lineage_columns: Vec<&str> = config
        .schema
        .metadata
        .iter()
        .filter(|field| field.generate_lineage_index)
        .map(|field| field.name.as_str())
        .collect();

    println!("Found {} columns with generateLineageIndex: true", lineage_columns.len());
    for column in &lineage_columns {
        println!("  - {}", column);
    }

    let mut lineage_definitions_downloaded = false;
    // Download lineage definitions for each column
    for column in lineage_columns {
        println!("\nDownloading lineage definition for column: {}", column);
        match download_lineage_definition(&client, &lapis_organism_url, column).await {
            Ok(content) => {
                let filename = format!("lineage_definition_{}.json", column);
                fs::write(&filename, &content)?;
                println!("Successfully saved to {}", filename);
                println!("File size: {} bytes", content.len());
            }
            Err(e) => {
                eprintln!("Failed to download lineage definition for {}: {}", column, e);
            }
        }
        lineage_definitions_downloaded = true;
    }

    // Download reference genomes
    println!("\nDownloading reference genomes...");
    match download_reference_genomes(&client, &lapis_organism_url).await {
        Ok(content) => {
            fs::write("reference_genomes.json", &content)?;
            println!("Successfully saved reference genomes to reference_genomes.json");
            println!("File size: {} bytes", content.len());
        }
        Err(e) => {
            eprintln!("Failed to download reference genomes: {}", e);
        }
    }

    // Download released data from backend
    println!("\nDownloading released data from backend...");
    match download_released_data(&client, &backend_organism_url).await {
        Ok(content) => {
            fs::write("released_data.json", &content)?;
            println!("Successfully saved released data to released_data.json");
            println!("File size: {} bytes", content.len());
        }
        Err(e) => {
            if e.to_string().contains("406 Not Acceptable") {
                println!("\n⚠️  The server returned '406 Not Acceptable' for the released data endpoint.");
                println!("This might indicate that the data needs to be downloaded manually.");
                println!("\nPlease manually download the released data from:");
                println!("  {}/get-released-data", backend_organism_url);
                println!("\nSave it as 'get-release-data.ndjson.zst' in the current directory.");
                println!("Press Enter to continue once you've downloaded the file...");

                // Wait for user input
                use std::io::{self, BufRead};
                let stdin = io::stdin();
                let _ = stdin.lock().lines().next();

                // Check if the file now exists
                if std::path::Path::new("get-release-data.ndjson.zst").exists() {
                    println!("✓ Found get-release-data.ndjson.zst, continuing...");
                } else {
                    println!("⚠️  File 'get-release-data.ndjson.zst' not found. Exiting.");
                    return Ok(());
                }
            } else {
                eprintln!("Failed to download released data: {}", e);
            }
        }
    }

    if args.transform_to_v8 {
        println!("\nTransforming released data to v8 format...");
        match transform_to_v8_format().await {
            Ok(_) => {
                println!("Successfully transformed data to v8 format: get-released-data.8.ndjson.zst");
            }
            Err(e) => {
                eprintln!("Failed to transform data to v8 format: {}", e);
            }
        }
    }

    // Generate preprocessing config
    println!("\nGenerating preprocessing_config.yaml...");
    generate_preprocessing_config(args.transform_to_v8, lineage_definitions_downloaded)?;

    Ok(())
}

fn generate_preprocessing_config(v8_file_available: bool, lineage_definitions_downloaded: bool) -> Result<(), Box<dyn std::error::Error>> {
    let mut config_lines = Vec::new();

    // Add ndjson input filename based on whether v8 transformation was successful
    if v8_file_available {
        config_lines.push("ndjsonInputFilename: get-released-data.8.ndjson.zst".to_string());
    } else {
        config_lines.push("ndjsonInputFilename: get-released-data.ndjson.zst".to_string());
    }

    // Add lineage definitions filename if any were downloaded
    if lineage_definitions_downloaded {
        config_lines.push("lineageDefinitionsFilename: lineage_definitions.yaml".to_string());
    } else {
        config_lines.push("# lineageDefinitionsFilename: lineage_definitions.yaml  # no lineage definitions downloaded".to_string());
    }

    let config_content = config_lines.join("\n");
    fs::write("preprocessing_config.yaml", config_content)?;

    println!("Successfully generated preprocessing_config.yaml");
    println!("Config contents:");
    for line in config_lines {
        println!("  {}", line);
    }

    Ok(())
}

async fn download_database_config(
    client: &Client,
    lapis_organism_url: &str
) -> Result<String, Box<dyn std::error::Error>> {
    println!("Downloading database config from: {}/databaseConfig", lapis_organism_url);

    // Make the GET request
    let response = client
        .get(&format!("{}/databaseConfig?downloadAsFile=false", lapis_organism_url))
        .header("accept", "application/yaml")
        .send()
        .await?;

    // Check if the request was successful
    if response.status().is_success() {
        // Get the response as text
        let body = response.text().await?;

        // Write the response to a file
        fs::write("database_config.yaml", &body)?;
        println!("Successfully downloaded and saved to database_config.yaml");
        println!("File size: {} bytes", body.len());

        Ok(body)
    } else {
        let error_msg = format!("Request failed with status: {}", response.status());
        println!("{}", error_msg);
        Err(error_msg.into())
    }
}

async fn download_lineage_definition(
    client: &Client,
    lapis_organism_url: &str,
    column: &str,
) -> Result<String, Box<dyn std::error::Error>> {
    let url = format!("{}/lineageDefinition/{}", lapis_organism_url, column);
    println!("Requesting: {}", url);

    let response = client
        .get(&url)
        .header("accept", "application/yaml")
        .send()
        .await?;

    if response.status().is_success() {
        let body = response.text().await?;

        // Write the response to a file
        fs::write("lineage_definitions.yaml", &body)?;
        println!("Successfully downloaded and saved to lineage_definitions.yaml");
        println!("File size: {} bytes", body.len());

        Ok(body)
    } else {
        Err(format!("Request failed with status: {} for column: {}", response.status(), column).into())
    }
}

async fn download_reference_genomes(
    client: &Client,
    lapis_organism_url: &str,
) -> Result<String, Box<dyn std::error::Error>> {
    let url = format!("{}/referenceGenome", lapis_organism_url);
    println!("Requesting: {}", url);

    let response = client
        .get(&url)
        .header("accept", "application/json")
        .send()
        .await?;

    if response.status().is_success() {
        let body = response.text().await?;

        // Write the response to a file
        fs::write("reference_genomes.json", &body)?;
        println!("Successfully downloaded and saved to reference_genomes.json");
        println!("File size: {} bytes", body.len());

        Ok(body)
    } else {
        Err(format!("Request failed with status: {} for reference genomes", response.status()).into())
    }
}



async fn download_released_data(
    client: &Client,
    backend_organism_url: &str
) -> Result<String, Box<dyn std::error::Error>> {
    let url = format!("{}/get-released-data", backend_organism_url);
    println!("Requesting: {}", url);

    let response = client
        .get(&url)
        .header("accept", "application/json")
        .send()
        .await?;

    if response.status().is_success() {
        let body = response.text().await?;
        Ok(body)
    } else {
        Err(format!("Request failed with status: {} for released data", response.status()).into())
    }
}

async fn transform_to_v8_format() -> Result<(), Box<dyn std::error::Error>> {
    println!("Running transformation pipeline...");

    // Check if required files exist
    if !std::path::Path::new("get-release-data.ndjson.zst").exists() {
        return Err("get-release-data.ndjson.zst not found".into());
    }

    if !std::path::Path::new("./tools/legacyNdjsonTransformer/Cargo.toml").exists() {
        return Err("./tools/legacyNdjsonTransformer/Cargo.toml not found".into());
    }

    // Build the pipeline command
    let output = Command::new("sh")
        .arg("-c")
        .arg("zstdcat get-release-data.ndjson.zst | cargo run --release --manifest-path ./tools/legacyNdjsonTransformer/Cargo.toml | zstd > get-released-data.8.ndjson.zst")
        .output()?;

    if output.status.success() {
        println!("Transformation completed successfully");
        if let Ok(stdout) = String::from_utf8(output.stdout) {
            if !stdout.is_empty() {
                println!("Output: {}", stdout);
            }
        }
    } else {
        let stderr = String::from_utf8_lossy(&output.stderr);
        return Err(format!("Transformation failed: {}", stderr).into());
    }

    Ok(())
}