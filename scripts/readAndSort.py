import sys
import re

def parse_cigar(cigar):
    pattern = re.compile(r'(\d+)([MIDNSHP=X])')
    
    parsed_cigar = pattern.findall(cigar)
    
    return [(op, int(length)) for length, op in parsed_cigar]


unpaired = dict()

sequence_graph = dict()

# have sequence_graph contain nodes forming clusters
# graph['79'] would be the starting node for all samples taken at position 79
# from that point on treat it as a prefix tree
# will have to compare to reference sequence to get diff array


known_sequences = dict()

with open('merged.fasta', 'w') as output_fasta, open('nuc_insertions.txt', 'w') as output_insertions, open('reference_genome.fasta', 'r') as reference_sequence_file:
    reference_sequence = reference_sequence_file.readlines()[1]
    # print("reference sequence: ", reference_sequence)
    for line in sys.stdin:
        if line.startswith('@'):
            continue
        
        fields = line.strip().split('\t')
        
        QNAME = fields[0]                   # Query template NAME
        FLAG = int(fields[1])               # bitwise FLAG
        RNAME = fields[2]                   # Reference sequence NAME
        POS = int(fields[3])                # 1-based leftmost mapping POSition
        MAPQ = int(fields[4])               # MAPping Quality
        CIGAR = parse_cigar(fields[5])      # CIGAR string
        RNEXT = fields[6]                   # Ref. name of the mate/next read
        PNEXT = int(fields[7])              # Position of the mate/next read
        TLEN = int(fields[8])               # observed Template LENgth
        SEQ = fields[9]                     # segment SEQuence
        QUAL = fields[10]                   # ASCII of Phred-scaled base QUALity + 33

        result_sequence = ''
        result_qual = ''
        index = 0
        inserts = []

        for operation in CIGAR:
            type, count = operation
            if type == 'S':
                index += count
                continue
            if type == 'M':
                result_sequence += SEQ[index:index + count]
                result_qual     += QUAL[index:index + count]
                index += count
                continue
            if type == 'D':
                result_sequence += '-' * count
                result_qual     += '!' * count
                continue
            if type == 'I':
                inserts.append((index + POS, SEQ[index:index + count]))
                index += count
                continue

        read = {
            # "QNAME": QNAME,
            # "FLAG": FLAG,
            # "RNAME": RNAME,
            "POS": POS,
            # "MAPQ": MAPQ,
            "CIGAR": CIGAR,
            # "RNEXT": RNEXT,
            # "PNEXT": PNEXT,
            # "TLEN": TLEN,
            # "SEQ": SEQ,
            # "QUAL": QUAL,
            "RESULT_SEQUENCE": result_sequence,
            "RESULT_QUAL": result_qual,
            "insertions": inserts,
        }

        if QNAME in unpaired:
            read1 = unpaired.pop(QNAME)
            read2 = read

            # print(read1)
            # print(read2)
            
            if read1['POS'] > read2['POS']:
                read1, read2 = read2, read1
            
            index = read1['POS']
            read1len = len(read1['RESULT_SEQUENCE'])
            merged = read1['RESULT_SEQUENCE'][:min(read1len, read2['POS'] - read1['POS'])]

            gaplen = read1['POS'] + read1len - read2['POS']
            if gaplen < 0:
                merged += 'N' * (-gaplen)
                merged += read2['RESULT_SEQUENCE']
            else:
                overlap_read1 = read1['RESULT_SEQUENCE'][read2['POS'] - read1['POS']:]
                overlap_read2 = read2['RESULT_SEQUENCE'][0: max(0, gaplen)]

                overlap_qual1 = read1['RESULT_QUAL'][read2['POS'] - read1['POS']:]
                overlap_qual2 = read2['RESULT_QUAL'][0: max(0, gaplen)]

                overlap_result = list(overlap_read1)

                if len(overlap_result) and overlap_read1 != overlap_read2:
                    # print("", QNAME)
                    if len(overlap_read1) != len(overlap_read2):
                        print("overlaps don't match in size")
                    number_of_diffs = 0
                    for i in range(len(overlap_read1)):
                        if overlap_read1[i] != overlap_read2[i]:
                            if overlap_qual1[i] == '-' and overlap_read2 != '-':
                                overlap_result[i] = overlap_read2[i]
                            if overlap_qual1[i] > overlap_qual2[i]:
                                overlap_result[i] = overlap_read2[i]
                            # print("diff in position ", i, ": ", overlap_read1[i], "/", overlap_read2[i])
                            number_of_diffs += 1
                            # print("corresponding qs ", i, ": ", overlap_qual1[i], "/", overlap_qual2[i])

                merged += "".join(overlap_result) + read2['RESULT_SEQUENCE'][max(0, gaplen):]



            if len(merged) != read2['POS'] + len(read2['RESULT_SEQUENCE']) - read1['POS']:
                raise Exception("Length mismatch")
            
            # output_fasta.write(f">{QNAME}|{read1['POS']}\n{merged}\n")

            merged_insertions = read1['insertions'].copy()
            insertion_index = read1['POS'] + read1len
            merged_insertions += [insert for insert in read2['insertions'] if insert[0] > insertion_index]

            output_insertions.write(f"{QNAME}\t{merged_insertions}\n")

            # time to add it to the graph
            reference_offset = read1['POS'] - 1

            # for i in range(len(merged)):
                # if merged[i] != reference_sequence[reference_offset + i]:
                #     # print("id: ", QNAME, "mutation at ", i, " from ", reference_sequence[reference_offset + i], " to ", merged[i])
            fingerprint = hash(merged)
            if fingerprint in known_sequences:
                groups = known_sequences[fingerprint]
                found = False
                for group in groups:
                    if group[0] == merged:
                        found = True
                        group[2].append(QNAME)
                if not found:
                    known_sequences[fingerprint].append((merged, read1['POS'], [QNAME]))
                
            else:
                known_sequences[fingerprint] = [(merged, read1['POS'], [QNAME])]

        else:
            unpaired[QNAME] = read

    print("Number of unique sequences: ", sum([len(groups) for groups in known_sequences.values()]))

    flattened = [group for groups in known_sequences.values() for group in groups]
    flattened.sort(key=lambda x: x[1])

    sort_number_prefix = 0

    for group in flattened:
        for id in group[2]:
            output_fasta.write(f">{sort_number_prefix:010}.{id}|{group[1]}\n{group[0]}\n")
            sort_number_prefix += 1

    for id in unpaired:
        output_fasta.write(f">{id}|{unpaired[id]['POS']}\n{unpaired[id]['RESULT_SEQUENCE']}\n")
        output_insertions.write(f"{id}\t{unpaired[id]['insertions']}\n")