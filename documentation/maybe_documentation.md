
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