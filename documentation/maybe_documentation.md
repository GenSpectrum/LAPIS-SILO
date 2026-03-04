
### `Maybe`

This filter is true iff its child expression is true, where,
for all `NucleotideEquals` or `AminoAcidEquals` expressions
contained in this filters subtree, _there exists at least
one_ possible non-ambiguous sequence symbols at the sequence
position, where the expression would evaluate to true.

### `Exact`

This filter is true iff its child expression is true, where,
for all `NucleotideEquals` or `AminoAcidEquals` expressions
contained in this filters subtree, the expression would
evaluate to true _for all_ possible non-ambiguous sequence
symbols at the sequence position.

## Ambiguity Symbol Graph

Ambiguous symbols are IUPAC codes that represent multiple possible concrete symbols.
Two maps define the ambiguity system:

- **`CODES_FOR[S]`**: The set of concrete symbols that `S` codes for (the primary definition).
- **`AMBIGUITY_SYMBOLS[S]`**: The set of all symbols `Y` such that `CODES_FOR[S] ⊆ CODES_FOR[Y]`. In other words, all symbols that are at least as general as `S`. This is derived from `CODES_FOR`.

### Nucleotide CODES_FOR Graph

```
Symbol -> Codes for
───────────────────────────────
GAP    -> {GAP}
A      -> {A}
C      -> {C}
G      -> {G}
T      -> {T}
R      -> {A, G}           (puRine)
Y      -> {C, T}           (pYrimidine)
S      -> {G, C}           (Strong)
W      -> {A, T}           (Weak)
K      -> {G, T}           (Keto)
M      -> {A, C}           (aMino)
B      -> {C, G, T}        (not A)
D      -> {A, G, T}        (not C)
H      -> {A, C, T}        (not G)
V      -> {A, C, G}        (not T)
N      -> {all symbols}    (aNy)
```

### Nucleotide AMBIGUITY_SYMBOLS (derived)

```
Symbol -> Matched by
───────────────────────────────
GAP    -> {GAP, N}
A      -> {A, R, M, W, D, H, V, N}
C      -> {C, Y, S, M, B, H, V, N}
G      -> {G, R, K, S, B, D, V, N}
T      -> {T, Y, K, W, B, D, H, N}
R      -> {R, D, V, N}
Y      -> {Y, B, H, N}
S      -> {S, B, V, N}
W      -> {W, D, H, N}
K      -> {K, B, D, N}
M      -> {M, H, V, N}
B      -> {B, N}
D      -> {D, N}
H      -> {H, N}
V      -> {V, N}
N      -> {N}
```

### Amino Acid CODES_FOR Graph

```
Symbol -> Codes for
───────────────────────────────
GAP    -> {GAP}
A      -> {A}              (Alanine)
C      -> {C}              (Cysteine)
D      -> {D}              (Aspartic Acid)
E      -> {E}              (Glutamic Acid)
F      -> {F}              (Phenylalanine)
G      -> {G}              (Glycine)
H      -> {H}              (Histidine)
I      -> {I}              (Isoleucine)
K      -> {K}              (Lysine)
L      -> {L}              (Leucine)
M      -> {M}              (Methionine)
N      -> {N}              (Asparagine)
O      -> {O}              (Pyrrolysine)
P      -> {P}              (Proline)
Q      -> {Q}              (Glutamine)
R      -> {R}              (Arginine)
S      -> {S}              (Serine)
T      -> {T}              (Threonine)
U      -> {U}              (Selenocysteine)
V      -> {V}              (Valine)
W      -> {W}              (Tryptophan)
Y      -> {Y}              (Tyrosine)
B      -> {D, N}           (Aspartic acid or Asparagine)
J      -> {L, I}           (Leucine or Isoleucine)
Z      -> {Q, E}           (Glutamine or Glutamic acid)
STOP   -> {STOP}
X      -> {all symbols}    (any amino acid)
```

### Amino Acid AMBIGUITY_SYMBOLS (derived)

```
Symbol -> Matched by
───────────────────────────────
GAP    -> {GAP, X}
A      -> {A, X}
C      -> {C, X}
D      -> {D, B, X}
E      -> {E, Z, X}
F      -> {F, X}
G      -> {G, X}
H      -> {H, X}
I      -> {I, J, X}
K      -> {K, X}
L      -> {L, J, X}
M      -> {M, X}
N      -> {N, B, X}
O      -> {O, X}
P      -> {P, X}
Q      -> {Q, Z, X}
R      -> {R, X}
S      -> {S, X}
T      -> {T, X}
U      -> {U, X}
V      -> {V, X}
W      -> {W, X}
Y      -> {Y, X}
B      -> {B, X}
J      -> {J, X}
Z      -> {Z, X}
STOP   -> {STOP, X}
X      -> {X}
```