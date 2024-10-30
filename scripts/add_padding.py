import sys

def transform_fasta():
    while True:
        header = sys.stdin.readline().strip()
        sequence = sys.stdin.readline().strip()

        if not header or not sequence:
            break  # End of file
        
        # Parse the header and extract the position offset
        header_parts = header.split('|')
        position_offset = int(header_parts[1])  # Get the position offset
        
        # Add Ns before and after the sequence
        left_padding = 'N' * position_offset
        right_padding = 'N' * (29904 - len(sequence) - position_offset)
        
        # Write the transformed sequence to stdout
        sys.stdout.write(f"{header_parts[0]}\n")
        sys.stdout.write(f"{left_padding}{sequence}{right_padding}\n")

# Execute the function
transform_fasta()
