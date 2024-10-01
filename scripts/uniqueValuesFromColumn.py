import csv
import argparse

def extract_unique_values(input_csv, column_idx, output_csv):
    unique_values = set()

    with open(input_csv, mode='r', newline='', encoding='utf-8') as infile:
        reader = csv.reader(infile)
        next(reader, None)

        for row_number, row in enumerate(reader, start=2):  # Start from row 2 considering header is row 1
            if len(row) <= column_idx:
                raise ValueError(f"Row {row_number} has fewer than {column_idx + 1} columns.")
            unique_values.add(row[column_idx])

    sorted_unique_values = sorted(unique_values)
    with open(output_csv, mode='w', newline='', encoding='utf-8') as outfile:
        writer = csv.writer(outfile)
        for value in sorted_unique_values:
            writer.writerow([value])

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Extract unique values from a column of a CSV file. Also strips header line")
    parser.add_argument("input_csv", help="Path to the input CSV file")
    parser.add_argument("column_idx", type=int, help="Index (0-based) of the column to extract")
    parser.add_argument("output_csv", help="Path to the output CSV file")

    args = parser.parse_args()

    extract_unique_values(args.input_csv, args.column_idx, args.output_csv)
