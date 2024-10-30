### add_padding.py

Used for adding padding to an example fasta file, mainly used for testing main

#### Usage:

`cat input_file.fasta | python add_padding.py > output_file.fasta`

`input_file`: fasta file with header containing offset | delimited

`output_file`: normal fasta file, each sequence padded by the respective amount of N's to the left/right (position info in the header is not in the output)

### read.py

Used for merging pairs of reads

#### Usage:
`cat input_file.fasta | python read.py`
`samtools view input_file.bam | python read.py`

`input_file`: sam file contents, could also read from bam with samtools

`output_file`: it makes two output files by itself, merged.fasta and nuc_insertions.txt, the former has the merged reads and it uses the fasta with | headers to describe the position/offset

### read_outputsam.py

Used mainly for testing with IGV

same usage as read.py, except the output it makes is merged.sam, it doesn't store the insertions

the sam entries it outputs are the actual sequences you would find in the merged.fasta when running read.py - the cigar is just M's

### readAndSort.py
for reordering, using hashing, not really efficient more of a proof of concept

same usage as read.py

Note: I don't think I use the reference genome here so feel free to take it out of the code
