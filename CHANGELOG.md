# Changelog

## [0.5.1](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.5.0...v0.5.1) (2025-01-07)


### Bug Fixes

* return an error if reserved value INT32_MIN is used in filter ([042646b](https://github.com/GenSpectrum/LAPIS-SILO/commit/042646be296d7a8d68736ad3b0d8da6715f51c92))

## [0.5.0](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.4.0...v0.5.0) (2024-12-19)


### ⚠ BREAKING CHANGES

* new command line / config system

### Features

* add materializationCutoff option ([8920d5f](https://github.com/GenSpectrum/LAPIS-SILO/commit/8920d5ff7af6906aa55f4a5b034e11c631df2955))
* new command line / config system ([2a29074](https://github.com/GenSpectrum/LAPIS-SILO/commit/2a2907493fd8ce28d93fb73ccfdc205396a17e35)), closes [#633](https://github.com/GenSpectrum/LAPIS-SILO/issues/633)


### Bug Fixes

* declare virtual constructor for abstract base class CustomSqlFunction ([603ddf6](https://github.com/GenSpectrum/LAPIS-SILO/commit/603ddf6f23595de390dac4899411177f95fec022))

## [0.4.0](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.3.2...v0.4.0) (2024-11-14)


### ⚠ BREAKING CHANGES

* `metadataInputFile` key in preprocessing config file has been removed. Instead, ndjson files should be used and specified with the `ndjsonInputFilename` option

### Features

* remove tsv/fasta/sam input format ([#562](https://github.com/GenSpectrum/LAPIS-SILO/issues/562)) ([fd7dc6f](https://github.com/GenSpectrum/LAPIS-SILO/commit/fd7dc6f4628f7dc512a99daeb184bab3e05d4150))


### Bug Fixes

* **preprocessing:** correct error message when field is in config but not in metadata file ([828182a](https://github.com/GenSpectrum/LAPIS-SILO/commit/828182af791ea4da1fd39685f3366b9f36bacb02))
* remove shared_ptr in lambda captures which might lead to memory leaks ([a26d8b8](https://github.com/GenSpectrum/LAPIS-SILO/commit/a26d8b88146e5ca7f2841525c1d8c3b6bc93e87b))

## [0.3.2](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.3.1...v0.3.2) (2024-10-30)


### Bug Fixes

* **preprocessing**: add column_name to column classes and add them to error messages ([e011e5c](https://github.com/GenSpectrum/LAPIS-SILO/commit/e011e5c804ed11a32bb42ca57be8d304c22d4fe9)) ([#629](https://github.com/GenSpectrum/LAPIS-SILO/pull/629))

## [0.3.1](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.3.0...v0.3.1) (2024-10-25)


### Bug Fixes

* **preprocessing:** error on duplicate primary keys ([687c3ef](https://github.com/GenSpectrum/LAPIS-SILO/commit/687c3ef8803c3dee19b222bcb790bbc4fb33cabe))
* validate the column types earlier to prevent duckdb auto-casting to cast types with differing round-trip conversions ([ac238b2](https://github.com/GenSpectrum/LAPIS-SILO/commit/ac238b208a02a7b00bdab92ff3039562e2a1d704))

## [0.3.0](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.24...v0.3.0) (2024-10-16)


### ⚠ BREAKING CHANGES

* generalized wildcard queries ([#458](https://github.com/GenSpectrum/LAPIS-SILO/issues/458))
  * The preprocessing config field `pangoLineageDefinitionFilename` has been renamed to `lineageDefinitionFilename`.
  * We now accept a YAML lineage definition file instead of a Pango alias key.
  * Input and query validation now checks whether the provided lineage exists in the defined lineages, and errors are thrown if validation fails.
  * The metadata type `pangoLineage` has been removed. `type: string` with `generateLineageIndex: true` should be used instead.


### Features

* generalized wildcard queries ([#458](https://github.com/GenSpectrum/LAPIS-SILO/issues/458)) ([1432320](https://github.com/GenSpectrum/LAPIS-SILO/commit/143232056146405c1fbb391d22adfd271f699c69))


### Bug Fixes

* resolve aliases when inserting to or querying lineage indexes again ([0e841e6](https://github.com/GenSpectrum/LAPIS-SILO/commit/0e841e659d2c59677820d3b83a21f864c27d069a))
* update script to also generate aliases ([43ac4aa](https://github.com/GenSpectrum/LAPIS-SILO/commit/43ac4aa0ffd6939fafa6b1f56a8ec1e7190e0c22))

## [0.2.24](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.23...v0.2.24) (2024-10-15)


### Bug Fixes

* prevent setDatabase starvation which leads to no updates of currently loaded database ([24e4db6](https://github.com/GenSpectrum/LAPIS-SILO/commit/24e4db60b6da7d5e7b909818d230a740aa41ef74)) ([#613](https://github.com/GenSpectrum/LAPIS-SILO/pull/613))

## [0.2.23](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.22...v0.2.23) (2024-10-10)


### Bug Fixes

* **preprocessing:** enforce exact match on file base stem when pairing sequence and metadata when processing from file ([7c9b23d](https://github.com/GenSpectrum/LAPIS-SILO/commit/7c9b23d618a75c4455ed9c7ac4cd76cf1218251a)), closes [#607](https://github.com/GenSpectrum/LAPIS-SILO/issues/607)

## [0.2.22](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.21...v0.2.22) (2024-10-01)


### Bug Fixes

* correctly escape quotes in field names ([#595](https://github.com/GenSpectrum/LAPIS-SILO/issues/595)) ([7e7b448](https://github.com/GenSpectrum/LAPIS-SILO/commit/7e7b448670c2080bee62c876adb68e6e439e9da6))

## [0.2.21](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.20...v0.2.21) (2024-09-20)


### Bug Fixes

* **preprocessing:** resolves spurious OOM crashes when handling large datasets ([6e4eae2](https://github.com/GenSpectrum/LAPIS-SILO/commit/6e4eae283317b932e34008912997bfd6f4037c10))

## [0.2.20](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.19...v0.2.20) (2024-09-17)


### Features

* make duckdb memory limit configurable via preprocessing config ([c112eb1](https://github.com/GenSpectrum/LAPIS-SILO/commit/c112eb18976b86240ae23069bf11b7286e634d1f))

## [0.2.19](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.18...v0.2.19) (2024-09-12)


### Bug Fixes

* preprocessing: validate that either a ndjson or tsv file was given as input ([#564](https://github.com/GenSpectrum/LAPIS-SILO/issues/564)) ([7bf2ef7](https://github.com/GenSpectrum/LAPIS-SILO/commit/7bf2ef7fafb63fe2938cf846385ddc087290ca3b))

## [0.2.18](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.17...v0.2.18) (2024-09-12)


### Features

* do not abort on assertment failures ([4e5f598](https://github.com/GenSpectrum/LAPIS-SILO/commit/4e5f59879ce95c8925e15ca6df2f773d989e5c51))
* panic: add `ASSERT`, `UNREACHABLE`, `UNIMPLEMENTED`, safer env var ([867d08f](https://github.com/GenSpectrum/LAPIS-SILO/commit/867d08f4f8b068617b2102edb1ddb36fa6c585fc))
* update duckdb version ([577c1b2](https://github.com/GenSpectrum/LAPIS-SILO/commit/577c1b29d0dde43caf23227b518b7a1927e1f9be))


### Bug Fixes

* duplicate key in sample.ndjson ([c9b5232](https://github.com/GenSpectrum/LAPIS-SILO/commit/c9b523221a8a1ee20ce8830ce8105f2a248afd13))
* increase duckdb memory limit to 80GB ([20131bb](https://github.com/GenSpectrum/LAPIS-SILO/commit/20131bbd294f1da3683a58e0fb7e58cc72649bd5))
* order of randomized tests changed ([d29b2f2](https://github.com/GenSpectrum/LAPIS-SILO/commit/d29b2f267274db5708e857b22240c782a3fa2e80))

## [0.2.17](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.16...v0.2.17) (2024-08-30)


### Bug Fixes

* manually overwrite already existing unaligned sequence files in the temp folder ([f37fd88](https://github.com/GenSpectrum/LAPIS-SILO/commit/f37fd88dc36f0907e09881f6ca921c405a99cf43))
  
  * In case the unaligned sequence directory already existed in the temp folder when running preprocessing,
  it is manually deleted. Before, this lead to a bug, as the currently used duckdb version does
  not correctly overwrite the temp files and may leave duplicate entries
  in the output. This is fixed in duckdb PR [#11787](https://github.com/duckdb/duckdb/pull/11787) 
  which is included in release v0.10.3

## [0.2.16](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.15...v0.2.16) (2024-08-29)


### Bug Fixes

* do not order keys and sequences differently, leading to wrong unaligned sequence results when dateToSortBy is set ([b0f8352](https://github.com/GenSpectrum/LAPIS-SILO/commit/b0f8352814594e6171217ebf4f869b41d51453a3))
* do not return incorrect results when encountering a null value in the unaligned sequence action ([a69d117](https://github.com/GenSpectrum/LAPIS-SILO/commit/a69d117fdce2f8263d450b9646ab4432f09da304))
* fix the RIGHT JOIN in preprocessing of unaligned sequences to contain the correct primary key ([a25d5d0](https://github.com/GenSpectrum/LAPIS-SILO/commit/a25d5d0527e5cb2571c1d94c73c7d5b8ebcee3b6))

## [0.2.15](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.14...v0.2.15) (2024-08-23)


### Bug Fixes

* don't abort when unaligned sequence is missing - we should return `null` ([#542](https://github.com/GenSpectrum/LAPIS-SILO/issues/542)) ([2c649cb](https://github.com/GenSpectrum/LAPIS-SILO/commit/2c649cb324b2855204869a9cead42b03e6378fdb)), closes [#541](https://github.com/GenSpectrum/LAPIS-SILO/issues/541)

## [0.2.14](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.13...v0.2.14) (2024-08-19)


### Features

* new operator for matching strings against regular expressions ([ade25e8](https://github.com/GenSpectrum/LAPIS-SILO/commit/ade25e8b147914ff7383a0404981437d77956dd3))


### Bug Fixes

* typo in the error message if a column is not found ([67029a4](https://github.com/GenSpectrum/LAPIS-SILO/commit/67029a4ace345b8c3687ea39aff066849bcc19d1))

## [0.2.13](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.12...v0.2.13) (2024-08-06)


### Features

* bump serialization version to 2 ([e897203](https://github.com/GenSpectrum/LAPIS-SILO/commit/e897203780e7cd7546dfc82a2d29a6725bf07da2))
* bump serialization version to 3 ([7584214](https://github.com/GenSpectrum/LAPIS-SILO/commit/7584214e7c6866d38ad415c82abf7a01aa95d8bf))
* support SAM files as sequence input and allow partial sequence input with an offset ([5be9a9f](https://github.com/GenSpectrum/LAPIS-SILO/commit/5be9a9f23d5c1e2c54ca5837a82798aa459f9859))


### Bug Fixes

* allow all sequence-names by escaping them properly in all SQL statements ([901fc7e](https://github.com/GenSpectrum/LAPIS-SILO/commit/901fc7e0645954c7fc5ca705fd9c3902ab3d0928))

## [0.2.12](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.11...v0.2.12) (2024-07-31)


### Features

* change base image to ubuntu ([64dee0d](https://github.com/GenSpectrum/LAPIS-SILO/commit/64dee0d90decd8aeafacbf88db611c53eb12c4b5))

## [0.2.11](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.10...v0.2.11) (2024-07-24)


### Bug Fixes

* streaming: report an error when sorting requested while streaming ([313d6bb](https://github.com/GenSpectrum/LAPIS-SILO/commit/313d6bba7b0201c330fe4b39a6d68063142f8b1d))

## [0.2.10](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.9...v0.2.10) (2024-07-22)


### Bug Fixes

* reduce function calls and parallelism when inserting rows into storage backend ([#509](https://github.com/GenSpectrum/LAPIS-SILO/issues/509)) ([93ba858](https://github.com/GenSpectrum/LAPIS-SILO/commit/93ba8584fed5807123ea3fb724ff55fc57b6265d))
* preprocessing: don't abort inserting insertions upon finding a null value ([03a500a](https://github.com/GenSpectrum/LAPIS-SILO/commit/03a500acf02f4d13975748f1bb6f222392604b7c))

## [0.2.9](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.8...v0.2.9) (2024-07-18)


### Features

* have no default sequence by default, implement default amino acid sequence from config ([2edf924](https://github.com/GenSpectrum/LAPIS-SILO/commit/2edf9240fb9f63400289121f495367c7aff4ea73)), closes [#454](https://github.com/GenSpectrum/LAPIS-SILO/issues/454)
* set the default sequence when there is only a single sequence in the reference genomes ([0ae4022](https://github.com/GenSpectrum/LAPIS-SILO/commit/0ae40221e66382ac330748c3eefe916de5e26975)), closes [#454](https://github.com/GenSpectrum/LAPIS-SILO/issues/454)
* throw error in preprocessing when default sequence is not in reference genomes ([287bb8b](https://github.com/GenSpectrum/LAPIS-SILO/commit/287bb8bb08f376a2b9a9c69613b73182c900fc7a)), closes [#454](https://github.com/GenSpectrum/LAPIS-SILO/issues/454)


### Bug Fixes

* streaming: report an error when the unimplemented offset or limit matters ([3be9c06](https://github.com/GenSpectrum/LAPIS-SILO/commit/3be9c06ded44c0b1666fd770d88876856a0a9899)), closes [#511](https://github.com/GenSpectrum/LAPIS-SILO/issues/511)

## [0.2.8](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.7...v0.2.8) (2024-07-08)


### Features

* the `Fasta` and `FastaAligned` actions now stream their result instead of allocating it in memory, closes [#112](https://github.com/GenSpectrum/LAPIS-SILO/issues/112)

## [0.2.7](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.6...v0.2.7) (2024-06-25)


### Bug Fixes

* don't abort when reading table in chunks ([26b1558](https://github.com/GenSpectrum/LAPIS-SILO/commit/26b155881a6f0a46ea1882c4f3f3641d691b13a7))

## [0.2.6](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.5...v0.2.6) (2024-06-19)


### Bug Fixes

* trigger release due to refactor commit-5e3041e5 ([#494](https://github.com/GenSpectrum/LAPIS-SILO/issues/494)) ([#496](https://github.com/GenSpectrum/LAPIS-SILO/issues/496)) ([ecd4ba0](https://github.com/GenSpectrum/LAPIS-SILO/commit/ecd4ba0f4b2f5338c4f968bda7b2b18dfbea0c2c))

## [0.2.5](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.4...v0.2.5) (2024-06-17)


### Bug Fixes

* empty input without partitioning ([5fa3c92](https://github.com/GenSpectrum/LAPIS-SILO/commit/5fa3c9267eff1b0e59dda718a2ded0f874b038ae))

## [0.2.4](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.3...v0.2.4) (2024-06-14)


### Features

* allow null for sequenceName in insertion contains queries ([6dbe251](https://github.com/GenSpectrum/LAPIS-SILO/commit/6dbe251a03e8a317188de0292f0e637b8ec4c24d))


### Bug Fixes

* more efficient ndjson emptiness check ([#481](https://github.com/GenSpectrum/LAPIS-SILO/issues/481)) ([344ec7b](https://github.com/GenSpectrum/LAPIS-SILO/commit/344ec7b20a3d6727e8972334542a610260cdc782))

## [0.2.3](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.2...v0.2.3) (2024-06-10)


### Bug Fixes

* be able to start without genes, without nucleotide sequences or with neither ([e878ed5](https://github.com/GenSpectrum/LAPIS-SILO/commit/e878ed5567679f966c615a26b2d4c713f2f045e6))
* empty ndjson input files more robust ([#473](https://github.com/GenSpectrum/LAPIS-SILO/issues/473)) ([9d4232b](https://github.com/GenSpectrum/LAPIS-SILO/commit/9d4232b01cb366def7e464155867bc6f7a466471))
* erroneous file created during unit tests should not leak ([1b764af](https://github.com/GenSpectrum/LAPIS-SILO/commit/1b764afbefe13969496bf8b2bf51d50824c495bf))
* insertions being added at wrong index for large files ([#472](https://github.com/GenSpectrum/LAPIS-SILO/issues/472)) ([e056ed9](https://github.com/GenSpectrum/LAPIS-SILO/commit/e056ed9354306690b51d6d0f829b962420e2be24))
* remove incorrect compile flags ([b92ee4e](https://github.com/GenSpectrum/LAPIS-SILO/commit/b92ee4e0816f0f5b9ef1bef711b968600620f80d))

## [0.2.2](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.1...v0.2.2) (2024-05-31)


### Bug Fixes

* start with empty files without throwing an error ([d407b92](https://github.com/GenSpectrum/LAPIS-SILO/commit/d407b9277ea89294ebbc581a80d23124040e266d))

## [0.2.1](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.2.0...v0.2.1) (2024-05-28)


### Bug Fixes

* allow insertions that start at 0 ([#449](https://github.com/GenSpectrum/LAPIS-SILO/issues/449)) ([f427137](https://github.com/GenSpectrum/LAPIS-SILO/commit/f4271375b743cab0b012ef4337316c8bb3d0c6d8))

## [0.2.0](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.1.1...v0.2.0) (2024-05-23)


### ⚠ BREAKING CHANGES

* old database_config files might be invalid if they contained insertion columns. Also, we are more prohibitive for ndjson input files, which now MUST contain nucleotide/amino acid insertions for all respective sequences. The insertions action and filter do no longer require a column field.

### Features

* insertions no longer in metadata or databaseConfig, instead expected for all aligned sequences [#372](https://github.com/GenSpectrum/LAPIS-SILO/issues/372) ([1f3680c](https://github.com/GenSpectrum/LAPIS-SILO/commit/1f3680c87e912b57a79cd1280213f828a40c238e))

## [0.1.1](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.1.0...v0.1.1) (2024-05-23)


### Features

* add number of partitions to /info ([1e780b5](https://github.com/GenSpectrum/LAPIS-SILO/commit/1e780b51f7b64334e9102fef3eeb279132e363e5))
* add serializationVersion to SILO output for smoother transitioning to new formats ([d3badb6](https://github.com/GenSpectrum/LAPIS-SILO/commit/d3badb65e6c2ce5d59ab8134cf2a237c836d417e))

## [0.1.0](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.0.1...v0.1.0) (2024-05-10)


### ⚠ BREAKING CHANGES

* return data as NDJSON instead of JSON

### Features

* AAMutations with multiple sequences ([0def8b2](https://github.com/GenSpectrum/LAPIS-SILO/commit/0def8b21e27cbc25973fb64ac9e56670c09cc023))
* Action for amino acid distribution ([a0a4cf1](https://github.com/GenSpectrum/LAPIS-SILO/commit/a0a4cf1e7422ef536c6e40863f27fdd85151cc65))
* add limit orderBy and offset to all query actions ([13b7e01](https://github.com/GenSpectrum/LAPIS-SILO/commit/13b7e01cbd4396e8e4cbb6357baec4514ad2eb6c))
* add log statements to loadDatabaseState ([46a0421](https://github.com/GenSpectrum/LAPIS-SILO/commit/46a04214244ba4ff799bcb2b9bf4c2cd1ad4e006))
* add more tests, make less flaky and viable with large dataset ([7772ae3](https://github.com/GenSpectrum/LAPIS-SILO/commit/7772ae345d44a2feb32126f87026f011e7e49a59))
* add unit test for findIllegalNucleotideChar, unique test case name for insertion contains invalid pattern tests ([99c9c4b](https://github.com/GenSpectrum/LAPIS-SILO/commit/99c9c4b3cd8935c2b2ba9998cf2e0847107edd4f))
* added amino acid insertion search, added many test cases and fixed various bugs ([d1e4b2b](https://github.com/GenSpectrum/LAPIS-SILO/commit/d1e4b2b757a23f81effe3e8c1be4a112f073d590))
* allow default preprocessing config along with user defined preprocessing config ([ee9f20e](https://github.com/GenSpectrum/LAPIS-SILO/commit/ee9f20edc58b55f6afd168888ed88f0faa486109))
* allow reading fasta files with missing segments and genes [#220](https://github.com/GenSpectrum/LAPIS-SILO/issues/220) ([8ea9893](https://github.com/GenSpectrum/LAPIS-SILO/commit/8ea9893b5f0fcc0135864eae69057118ff5375dd))
* allow reading segments and genes that are null from ndjson file [#220](https://github.com/GenSpectrum/LAPIS-SILO/issues/220) ([d0a3a7e](https://github.com/GenSpectrum/LAPIS-SILO/commit/d0a3a7e0f6eb2c8d45e400bee183889d4cd41e4e))
* also get Runtime Config options from environment variables ([33bdd65](https://github.com/GenSpectrum/LAPIS-SILO/commit/33bdd65b826f97d682aed9f4f98c18870220e60a))
* also log to stdout ([54b8a47](https://github.com/GenSpectrum/LAPIS-SILO/commit/54b8a472316ad5230bcec14e0e0abfab2b7caba2))
* also return mutation destructed that does not need to be reparsed ([a93abbf](https://github.com/GenSpectrum/LAPIS-SILO/commit/a93abbf3b294a61f5858b34a140a3e6b9b228657))
* Alternative templating of symbol classes ([6b61985](https://github.com/GenSpectrum/LAPIS-SILO/commit/6b619858b8651914098a850a9bbed877921a4cd7))
* automatically detect file endings for fasta files ([75bd14e](https://github.com/GenSpectrum/LAPIS-SILO/commit/75bd14e87531e3f3d6c08d7bc3dea8158a4311e1))
* be more lenient on input data, ignore superfluous sequences and fill missing sequences with Ns ([ee12186](https://github.com/GenSpectrum/LAPIS-SILO/commit/ee1218670d4cea5c4599c5c42b0c913d9eb8b864))
* Better test coverage for SymbolEquals filter ([42c685c](https://github.com/GenSpectrum/LAPIS-SILO/commit/42c685cf4c8ce068072fbf35ac8b6a0e263234f9))
* **boolean columns, resolves #384:** const declaration ([685db9f](https://github.com/GenSpectrum/LAPIS-SILO/commit/685db9fea3562076e2dbcb9dbf426154fbd60eb7))
* **boolean columns:** actions/tuple: update assignTupleField() ([b020109](https://github.com/GenSpectrum/LAPIS-SILO/commit/b020109a7c25908b2ef647c94eb240d9a92ba8c1))
* **boolean columns:** add and use JsonValueType ([2ad1268](https://github.com/GenSpectrum/LAPIS-SILO/commit/2ad1268c36fd6c4dbd926db0adbeef0a5cf28127))
* **boolean columns:** add bool to JsonValueType, update tuple ([3c990e2](https://github.com/GenSpectrum/LAPIS-SILO/commit/3c990e25f4e42c802d9552e19342edfc6fc2f378))
* **boolean columns:** add expression_type "BooleanEquals" ([b82ec69](https://github.com/GenSpectrum/LAPIS-SILO/commit/b82ec69e5d7aef6b75dc68de6290e1d962b0edf5))
* **boolean columns:** add filter_expressions/bool_equals ([ff2a138](https://github.com/GenSpectrum/LAPIS-SILO/commit/ff2a138a82670ba3035c7432c0c558d520d79268))
* **boolean columns:** add optional_bool ([a2e47f8](https://github.com/GenSpectrum/LAPIS-SILO/commit/a2e47f8d1895e852d487be0ed3a88fa5a928f4f4))
* **boolean columns:** add storage/column/bool_column ([5eb3430](https://github.com/GenSpectrum/LAPIS-SILO/commit/5eb3430e3eafe5ab28dbf90632f1bb85663d66e8))
* **boolean columns:** column_group: update ColumnPartitionGroup ([d7adecf](https://github.com/GenSpectrum/LAPIS-SILO/commit/d7adecf4e9c51bb9e3e6b1aad8e3f7b43e0692b0))
* **boolean columns:** column_group.h: add {ColumnPartitionGroup,ColumnGroup}::bool_columns fields ([af44f1a](https://github.com/GenSpectrum/LAPIS-SILO/commit/af44f1a8cf552d8ab6625d9fd440d83b2e10c36f))
* **boolean columns:** database ([c555a68](https://github.com/GenSpectrum/LAPIS-SILO/commit/c555a68801db08d089be8d337f40a8793ac54b48))
* **boolean columns:** database_config: add "bool" case to DatabaseConfigReader::readConfig() ([12fc7b8](https://github.com/GenSpectrum/LAPIS-SILO/commit/12fc7b8c95decfd58692332a9496e7046ca12242))
* **boolean columns:** database_config: add "boolean" case to de/serialisation ([e6d5363](https://github.com/GenSpectrum/LAPIS-SILO/commit/e6d536330fb4ac162db34f5c5233ae495211da3e))
* **boolean columns:** database_config: add BOOL to ValueType ([0b47b82](https://github.com/GenSpectrum/LAPIS-SILO/commit/0b47b828915b293a1ed6c1210b3574045a6fcf67))
* **boolean columns:** database_config: update DatabaseMetadata::getColumnType() ([30febdd](https://github.com/GenSpectrum/LAPIS-SILO/commit/30febddb8914d1e909b10d7f31a0803b4a485ba7))
* **boolean columns:** database_partition ([0aef8f0](https://github.com/GenSpectrum/LAPIS-SILO/commit/0aef8f0c4e28fa8bdeec9f6638388e62d9b7b9cc))
* **boolean columns:** optional_bool: add `==` ([f0aa3e8](https://github.com/GenSpectrum/LAPIS-SILO/commit/f0aa3e8c1ac5b43d9414a84587bfb13ed818323e))
* **boolean columns:** selection ([e52f2dc](https://github.com/GenSpectrum/LAPIS-SILO/commit/e52f2dc5623929395d40aff3264c48099f6a246a))
* build metadata in parallel to sequences. Do not create unaligned sequence tables in preprocessing, rather hive-partition them directly to disk. Better (debug-)logging ([c1cdfeb](https://github.com/GenSpectrum/LAPIS-SILO/commit/c1cdfeb3cb25e8bcbd5e095e5dbc0f78ae35492f))
* bulk Tuple allocations now possible ([902ec04](https://github.com/GenSpectrum/LAPIS-SILO/commit/902ec0461544fc0cbc0e8e0ba07b18600b29126d))
* clearer Operator::negate and Expression::toString, logical Equivalents for debug printing/logging for the Leaf Operators IndexScan and BitmapSelection ([026b639](https://github.com/GenSpectrum/LAPIS-SILO/commit/026b639af5d45690c42c70b13b926ce72fdcc837))
* consistent behavior of configs when starting SILO with both --preprocessing and --api ([847ec7e](https://github.com/GenSpectrum/LAPIS-SILO/commit/847ec7e8af6eda73c95076c9b4428ed3fd5c3f9f))
* declutter README.md from linting option, which is now disabled by default and enforced in the CI for the Linter ([9220435](https://github.com/GenSpectrum/LAPIS-SILO/commit/922043538a39992b2322c02434826dab070d1538))
* details no longer shows insertions ([#354](https://github.com/GenSpectrum/LAPIS-SILO/issues/354)) ([473cd98](https://github.com/GenSpectrum/LAPIS-SILO/commit/473cd980f6aee5267e8d4d0cd65671e5722dc26b))
* display database info after loading new database state ([0249416](https://github.com/GenSpectrum/LAPIS-SILO/commit/02494165930eefbafb0f74e069962a94f830d3e8))
* display preprocessing duration  in logs in human-readable format (not in microseconds) [#296](https://github.com/GenSpectrum/LAPIS-SILO/issues/296) ([a2499af](https://github.com/GenSpectrum/LAPIS-SILO/commit/a2499af5cf3c5f46c695f019fda632c28939c62c))
* do not enforce building with clang-tidy by default. Linter will still be enforced ([7134e45](https://github.com/GenSpectrum/LAPIS-SILO/commit/7134e45ae2c66acb0e53ac433a187cdc4e2ccefa))
* FastaAligned action ([50776c8](https://github.com/GenSpectrum/LAPIS-SILO/commit/50776c83d8cb4409e4f01c6b49a189647f5140de))
* faster builds by copying [@corneliusroemer](https://github.com/corneliusroemer) image caching for our dependency images, which rarely change ([#374](https://github.com/GenSpectrum/LAPIS-SILO/issues/374)) ([7867bc7](https://github.com/GenSpectrum/LAPIS-SILO/commit/7867bc744648f5410793eef8d52df24b23c3c918))
* filter for amino acids ([b52aabd](https://github.com/GenSpectrum/LAPIS-SILO/commit/b52aabde38ef4e1fb976a37c89fe1869f03ace57))
* fix sorting ([1ed18ae](https://github.com/GenSpectrum/LAPIS-SILO/commit/1ed18ae4ab38fb4822b075e5c3deeac90b2bfb37))
* flipped bitmap can now be set before insertion ([f61c803](https://github.com/GenSpectrum/LAPIS-SILO/commit/f61c803f74d135e75865aa09a9b373aa3603f19d))
* format DatabaseConfig ([4fb8f1b](https://github.com/GenSpectrum/LAPIS-SILO/commit/4fb8f1b90486abc84d7d9e941fb8d31fa3cc3e5d))
* format PreprocessingConfig ([ee35207](https://github.com/GenSpectrum/LAPIS-SILO/commit/ee35207e29888d52cbbfba0ae4f4fe0dda80be1d))
* generalize mutations action to have consistent behavior for different symbols ([9834aea](https://github.com/GenSpectrum/LAPIS-SILO/commit/9834aea8413fe34efb5414178de8a93f4fbf076f))
* generalizing symbol and mutation filters. Clear handling of ambiguous symbols ([aa9ad4d](https://github.com/GenSpectrum/LAPIS-SILO/commit/aa9ad4df935eed84f482cb9d81bafaca6f3374a3))
* Generalizing the config for multiple nucleotide sequences and multiple genes ([9a80204](https://github.com/GenSpectrum/LAPIS-SILO/commit/9a802049ea95fb30955f12d9696ac672bd14ce21))
* have structured and destructured insertion in insertions response ([0a7e46a](https://github.com/GenSpectrum/LAPIS-SILO/commit/0a7e46ad0e7740919059773c327dcda2cb1ae09e))
* hide intermediate results of the preprocessing - don't put it in the output ([44327b0](https://github.com/GenSpectrum/LAPIS-SILO/commit/44327b0b30203c526fe7a3e40e1701645808940e))
* implement basic request id to trace requests [#303](https://github.com/GenSpectrum/LAPIS-SILO/issues/303) ([4defb59](https://github.com/GenSpectrum/LAPIS-SILO/commit/4defb59cfe9d48635592ae0a4cd572d6caa94895))
* implement data updates at runtime. More resilient to superfluous or missing directory separators ([dc5dfaa](https://github.com/GenSpectrum/LAPIS-SILO/commit/dc5dfaa50d13512dc39c665aeab20630fa94415c))
* implement insertion columns and search ([9167236](https://github.com/GenSpectrum/LAPIS-SILO/commit/91672365f8cfdfbecd71804e3064134f93adc1cb))
* improve loadDB speeds ([2b7cd7d](https://github.com/GenSpectrum/LAPIS-SILO/commit/2b7cd7dacadb5e3887225aa84b57da8f7ca84394))
* improve validation error message of some actions on orderByFields ([a0da5b5](https://github.com/GenSpectrum/LAPIS-SILO/commit/a0da5b54a1ca2e9eff9c718028bea56fc8cd2661))
* insertion action targets all insertion columns by default ([6b70241](https://github.com/GenSpectrum/LAPIS-SILO/commit/6b70241db3577480078e51e7bc861579013dd4e6))
* insertion columns for amino acids and multiple sequence names ([3cc8fee](https://github.com/GenSpectrum/LAPIS-SILO/commit/3cc8fee9153ec1b23a819c4331b3f9dce3e16b37))
* insertions action ([e067062](https://github.com/GenSpectrum/LAPIS-SILO/commit/e0670629b1a40dc0e56a31e15eaa42e51a350d66))
* insertions contains action now targets all columns if the column name is missing ([32a6951](https://github.com/GenSpectrum/LAPIS-SILO/commit/32a69514b654a737478f0aafe9cdca9c41cc0691))
* introduce new storage type for Sequence Positions, where the most numerous symbol is deleted ([6e15204](https://github.com/GenSpectrum/LAPIS-SILO/commit/6e15204a9265e65e26562c227f5ccaf5cfba8c65))
* introduce storage of unaligned sequences from either ndjson file or fasta file and make them queryable via the Fasta action ([44df849](https://github.com/GenSpectrum/LAPIS-SILO/commit/44df849d6f84eba3321285bdaf6f22509c0c5f17))
* load table lazily. Unaligned Sequences do not need to load the table ([c2a8439](https://github.com/GenSpectrum/LAPIS-SILO/commit/c2a8439fc724452c4184f8c49b3db65c6a99b731))
* log databaseConfig and preprocessingConfig ([d2dc58c](https://github.com/GenSpectrum/LAPIS-SILO/commit/d2dc58cdbed9cea1e9e3028324eaeef32d82dbba))
* logging for partition ([e75a925](https://github.com/GenSpectrum/LAPIS-SILO/commit/e75a925d8b4ac7bd66229437d244089d26cb10de))
* logging improvements ([4c12a88](https://github.com/GenSpectrum/LAPIS-SILO/commit/4c12a882aa75025fac99e65ba3a491f72a2b78ec))
* make database serializable again ([2523e67](https://github.com/GenSpectrum/LAPIS-SILO/commit/2523e67d08cc59c297bc4ad0a9134f1b96d2cb0a))
* make pangoLineageDefinitionFilename in preprocessing config optional, linter errors ([0f3dc53](https://github.com/GenSpectrum/LAPIS-SILO/commit/0f3dc534891f4d6553385e85de8ad77565f2a9f9))
* make partition_by field in config optional ([3942418](https://github.com/GenSpectrum/LAPIS-SILO/commit/394241812df369a908c5d4495e503eca3abdf3db))
* make SILO Docker image by default read data from /data ([e83b910](https://github.com/GenSpectrum/LAPIS-SILO/commit/e83b910d8f9c7e38fa7c80ee4571b85601f99ab4))
* make threads and max queued http connections available through optional parameter ([3ecde68](https://github.com/GenSpectrum/LAPIS-SILO/commit/3ecde686b4c6979d07b8dce73b2ccb8fda6b48ee))
* migration to duckdb 0.10.1 ([c1426ef](https://github.com/GenSpectrum/LAPIS-SILO/commit/c1426ef906da8d8914718eaa5173a1e7750d407b))
* mine data version at beginning of preprocessing ([362fe0f](https://github.com/GenSpectrum/LAPIS-SILO/commit/362fe0f988e9901ba5b48a5b0c1a81d480093eb8))
* More robust InputStreamWrapper ([305dd36](https://github.com/GenSpectrum/LAPIS-SILO/commit/305dd366422131d6ecfbb4ef97f3cb3edb712211))
* multiple performance improvements for details endpoint ([28f41d0](https://github.com/GenSpectrum/LAPIS-SILO/commit/28f41d01933a53095bf69888beef1ee5ebbc6208))
* optimize bitmaps before finishing partition ([5b06d58](https://github.com/GenSpectrum/LAPIS-SILO/commit/5b06d582859ec9dd1bb0c63ff5c52b21fa08b9fe))
* order all actions by default ([a2f5c04](https://github.com/GenSpectrum/LAPIS-SILO/commit/a2f5c04b64e9b175ea023cd82d7fe8f1bacf2f95))
* preparation of insertion columns ([c14a370](https://github.com/GenSpectrum/LAPIS-SILO/commit/c14a370ebf19800e9ad0b6eeb9e4957dafd77cba))
* put output and logs to gitignore ([789e489](https://github.com/GenSpectrum/LAPIS-SILO/commit/789e489980be9f65c618822c6460588c9ed7aff7))
* reenable bitmap inversion ([75ac20f](https://github.com/GenSpectrum/LAPIS-SILO/commit/75ac20f679c433f7b439ec4e9935f7ea005d9ca8))
* reenable pushdown of And expressions through selections ([802bec0](https://github.com/GenSpectrum/LAPIS-SILO/commit/802bec0cf1ba876138641b9c0702dde77ea01683))
* refactor saving and loading database to not require preprocessing structs anymore ([45bf7ed](https://github.com/GenSpectrum/LAPIS-SILO/commit/45bf7ede2130ec9d2cfc3c8d1c48d32a921e6e83))
* reintroduce randomize for all query actions ([166045c](https://github.com/GenSpectrum/LAPIS-SILO/commit/166045cad4953dfd1eb8c3ff2b72b2d903d2193b))
* reserve space in columns when bulk inserting rows ([e3c9620](https://github.com/GenSpectrum/LAPIS-SILO/commit/e3c962095152cdb427bd5ee2f6a748bb4788ef73))
* return data as NDJSON instead of JSON ([c236ba4](https://github.com/GenSpectrum/LAPIS-SILO/commit/c236ba43af666758a0c59083c3bd97396a6e4fec)), closes [#126](https://github.com/GenSpectrum/LAPIS-SILO/issues/126)
* return data version on each query ([be5c886](https://github.com/GenSpectrum/LAPIS-SILO/commit/be5c886e4dcdf8d1094ee28a08738f62a4ededb4))
* return only aliased pango lineages ([abf0844](https://github.com/GenSpectrum/LAPIS-SILO/commit/abf08444677ebd796dc2fad6107051c571a221ff))
* reveal some more details when reading YAML fails ([1f8d9db](https://github.com/GenSpectrum/LAPIS-SILO/commit/1f8d9db929add1120169b73c80b11700c88d4cfc))
* run preprocessing in github ci ([f53eddb](https://github.com/GenSpectrum/LAPIS-SILO/commit/f53eddbbbc6e24d1ee371cac4c752bae19ebaeb0))
* save database state into folder with name &lt;data version&gt; ([41923eb](https://github.com/GenSpectrum/LAPIS-SILO/commit/41923eb1c99ab1373d448a9fbdaa8219ddd4df92))
* separate preprocessing and starting silo ([9808e2d](https://github.com/GenSpectrum/LAPIS-SILO/commit/9808e2d91cece622121e6d47734b1901b76cdeef))
* serialization of partition descriptor to json ([472c1da](https://github.com/GenSpectrum/LAPIS-SILO/commit/472c1da51d66b79ea8e20aaa59d1e2856f333756))
* some suggestions for the insertion search ([ae900da](https://github.com/GenSpectrum/LAPIS-SILO/commit/ae900da3672e30d6c04ad4edf56e9b8d3195d13b))
* Specifiable nucSequence query target ([7cc609f](https://github.com/GenSpectrum/LAPIS-SILO/commit/7cc609fe716116ab0a77443eaa2dce73482b6116))
* statically disable deleted symbol optimisations because of performance penalty ([4e522f4](https://github.com/GenSpectrum/LAPIS-SILO/commit/4e522f41fbc5bccb14a505f38af83c96e17634fc))
* stick to the default of having config value keys in camel case ([a1cae40](https://github.com/GenSpectrum/LAPIS-SILO/commit/a1cae40d7e5163289bbf68f6398a036a59eb0cae))
* storing amino acids ([f11a330](https://github.com/GenSpectrum/LAPIS-SILO/commit/f11a330302d61eff2dfa2354c0d565faae89b400))
* support for nullable columns ([6f78e3a](https://github.com/GenSpectrum/LAPIS-SILO/commit/6f78e3ad61af29e20b98bda72d4edc34951e7c1b))
* support recombinant lineages ([3e848a5](https://github.com/GenSpectrum/LAPIS-SILO/commit/3e848a539f2b4970b071028f347e654c9b745149))
* template class for sequence store ([cef4d48](https://github.com/GenSpectrum/LAPIS-SILO/commit/cef4d487db0ef57b2aab5ffa339a5abec05f65cd))
* templatized Symbol classes ([6b9d734](https://github.com/GenSpectrum/LAPIS-SILO/commit/6b9d734d49fe2ae0ca47065361f6d6465166177c))
* test set with amino acid insertions ([de2c4f8](https://github.com/GenSpectrum/LAPIS-SILO/commit/de2c4f83d8b5d45cd9d0944f496b7a2f5b7d763f))
* throw an error when there is not initialized database loaded yet [#295](https://github.com/GenSpectrum/LAPIS-SILO/issues/295) ([b17f72a](https://github.com/GenSpectrum/LAPIS-SILO/commit/b17f72a55c7835ef5c07409ce7f0d742661a859c))
* tidying up CLI and file configuration for runtime config. Added option for specifying the port ([c3a88a0](https://github.com/GenSpectrum/LAPIS-SILO/commit/c3a88a0f44e6d1eaaf715ade8389f13e7a87fb40))
* Unit tests for Tuple ([4fc06e8](https://github.com/GenSpectrum/LAPIS-SILO/commit/4fc06e81de653dac470c02fd258eaf4532f876ae))
* update conan version ([5540a67](https://github.com/GenSpectrum/LAPIS-SILO/commit/5540a67f4054f5f985791b283593f2bdd6885232))
* use 'pragma once' as include guards instead of 'ifndef...' ([bc49aa5](https://github.com/GenSpectrum/LAPIS-SILO/commit/bc49aa5530c97ca7cc10f7da4c27e29cb6ee16c9))
* use own scope for preprocessing ([2a93846](https://github.com/GenSpectrum/LAPIS-SILO/commit/2a9384651ba545265a5c8ccc6feccc1249b44ffd))
* use same default min proportions for mutations actions as the old LAPIS ([f42f830](https://github.com/GenSpectrum/LAPIS-SILO/commit/f42f83088e2302cf604d8846b69ea7e58659570a))


### Bug Fixes

* adapt randomize query results to target architecture. x86 and ARM have possibly different std::hash results ([#355](https://github.com/GenSpectrum/LAPIS-SILO/issues/355)) ([600000d](https://github.com/GenSpectrum/LAPIS-SILO/commit/600000d80616bb1688eb4fc8a7855b361a1eae1d))
* add bash dependency which is required by conan build of pkgconf and is not installed on alpine by default ([1b3f51c](https://github.com/GenSpectrum/LAPIS-SILO/commit/1b3f51c5e7e6a88c5b7500f3fd832eead62fff3b))
* add insertion to database_config test ([aec40d0](https://github.com/GenSpectrum/LAPIS-SILO/commit/aec40d0f8366c8f15906c6edbed35439b5a794a0))
* add missing file for test ([262bedc](https://github.com/GenSpectrum/LAPIS-SILO/commit/262bedc35fc3704854b2f321c8cab49e08d803a3))
* add missing sequenceName field to mutation action "orderBy" ([06c8c86](https://github.com/GenSpectrum/LAPIS-SILO/commit/06c8c86ebeb822d03f46342faafcf1f7d29a7d51))
* add sleep statement before row call ([8fa8efb](https://github.com/GenSpectrum/LAPIS-SILO/commit/8fa8efb5f7634e9eb91ad71d0d5d3af47c0fcf30))
* add workaround so insertions are read correctly ([8a2bfa8](https://github.com/GenSpectrum/LAPIS-SILO/commit/8a2bfa8ee211a8e8ec6c2e83671eab8abe69fc87))
* allow sql keywords for metadata field names [#259](https://github.com/GenSpectrum/LAPIS-SILO/issues/259) ([6fbeee5](https://github.com/GenSpectrum/LAPIS-SILO/commit/6fbeee56732d2d3934ff5769cc07ded2cbf2caa4))
* also consider 'missing' symbols in the mutation action. Bugfix where Position invariant was broken because of 'missing' symbols ([fab72a6](https://github.com/GenSpectrum/LAPIS-SILO/commit/fab72a60795568d0d1dddc25bf4a8e594f77477b))
* alternative non-exhaustive three mer index ([467086f](https://github.com/GenSpectrum/LAPIS-SILO/commit/467086f52d75ecee39a2fcb7937554869a7d03e1))
* always build dependency image for amd64 platform ([4f73cb0](https://github.com/GenSpectrum/LAPIS-SILO/commit/4f73cb04687ec57df32878e310139671a110ed5a))
* bug when filtering for indexedStringColumns which are not present in some partitions ([62d4e08](https://github.com/GenSpectrum/LAPIS-SILO/commit/62d4e08d3718a4b4b20e283602b558ad26c23a13))
* bug where sequence reconstruction is false when the flipped bitmap is different from the reference sequence symbol ([edac58c](https://github.com/GenSpectrum/LAPIS-SILO/commit/edac58cb340c409f3db96f5e6d6c325c8fb9ecb7))
* change random ordering for gcc hashing ([78055ff](https://github.com/GenSpectrum/LAPIS-SILO/commit/78055ff25fef2541b260285552e416433c4b6ca2))
* change test to reflect new optimisations ([cb63010](https://github.com/GenSpectrum/LAPIS-SILO/commit/cb630104c81e1846d8d87de0704a90216fbe5986))
* compiling And: append selection_child-&gt;predicates to predicates - not vice versa ([bdebca5](https://github.com/GenSpectrum/LAPIS-SILO/commit/bdebca5caab90ce4f37c7cf7c7844fc7808b2ba6))
* deterministic order for e2e test ([02427a2](https://github.com/GenSpectrum/LAPIS-SILO/commit/02427a25d291f3aba8fcaa95e577bb32c9c32179))
* divergence between mac and linux info test results, fix memory leaks in Threshold.cpp ([ffcefd0](https://github.com/GenSpectrum/LAPIS-SILO/commit/ffcefd0fb82345dddce170c0254fb20a167e18ba))
* do not exclude zstd filter from boost installation ([c974230](https://github.com/GenSpectrum/LAPIS-SILO/commit/c97423091d073a501499e30d0ee3a6782da4ba3c))
* do not use std::filesystem::path::relative_path() to also support absolute paths ([b9ff422](https://github.com/GenSpectrum/LAPIS-SILO/commit/b9ff422b76e63a673b8869725dc3a3fec677979a))
* endToEndResults ([c807a33](https://github.com/GenSpectrum/LAPIS-SILO/commit/c807a33b11ceb3e2db81f14613cc7601e81ea689))
* error when the Mutations action looked for sequences but the filter was empty ([91e5d52](https://github.com/GenSpectrum/LAPIS-SILO/commit/91e5d52e0dc6d11522c8ea2cc0be237d332d967c))
* fix memory leaks in indexed_string_column.cpp and insertion_contains.cpp ([c2eeac8](https://github.com/GenSpectrum/LAPIS-SILO/commit/c2eeac88ec688f25e65409490dcf68b954e53789))
* floatEquals and floatBetween with null values ([47b436e](https://github.com/GenSpectrum/LAPIS-SILO/commit/47b436e611e2266226c3070d11dcbc3366de2c7c))
* hide nucleotide sequence for default sequence ([584715c](https://github.com/GenSpectrum/LAPIS-SILO/commit/584715cd210f51c4e24a2b301145a1bdf4a91ac5))
* insertion column, remove reference ([d940c52](https://github.com/GenSpectrum/LAPIS-SILO/commit/d940c52a206ab9884f707baa491c4b12c59dfe50))
* insertion search e2e and insertion column tests, dont allow non-empty value for insertion search ([19af04d](https://github.com/GenSpectrum/LAPIS-SILO/commit/19af04dd7fbcafa6004ddc4c2894967d427a8308))
* linking error on linux ([ad9076c](https://github.com/GenSpectrum/LAPIS-SILO/commit/ad9076c8f385c5d56f20219422ee1d1d49602966))
* linter ([54d3c34](https://github.com/GenSpectrum/LAPIS-SILO/commit/54d3c3435a38dbfa0ddbfaf5af01b03f2dbbfaa1))
* linter ([e6b6ab7](https://github.com/GenSpectrum/LAPIS-SILO/commit/e6b6ab70b89ce7065185d2a19c0017d93d40801b))
* linter ([b31851c](https://github.com/GenSpectrum/LAPIS-SILO/commit/b31851c44d2473560ef486659bd3f3aad1f40709))
* linter ([34830ab](https://github.com/GenSpectrum/LAPIS-SILO/commit/34830ab0f0818c07f19ad28f8aa12b4229018fca))
* linter errors ([e9e1bbf](https://github.com/GenSpectrum/LAPIS-SILO/commit/e9e1bbfea06510b2dbb1c1097daadab90ade3ab1))
* Linter throws again and added clang-format option ([87cb4a5](https://github.com/GenSpectrum/LAPIS-SILO/commit/87cb4a57422773c5117edc8cba9afdea5754f1b6))
* Make C++ flags in CMake compatible for MacOS ([38cae69](https://github.com/GenSpectrum/LAPIS-SILO/commit/38cae694f86abad05b650756091e2238c02baf72))
* metadata info test accessing getMetadataFields output no longer directly but over the address of a const ([0dcdee1](https://github.com/GenSpectrum/LAPIS-SILO/commit/0dcdee1f8b0f788d850dd9576e6d9f770969d440))
* missing include ([0773999](https://github.com/GenSpectrum/LAPIS-SILO/commit/0773999ae2178dc2b5e221cf74b5ffd7e523f06a))
* new linter errors ([ea6934c](https://github.com/GenSpectrum/LAPIS-SILO/commit/ea6934cb3daf6b8cdbcb707a113ed3866f87e6cd))
* no longer have regression when no bitmap flipped is most efficient ([b653753](https://github.com/GenSpectrum/LAPIS-SILO/commit/b65375320612625fd1a7325878174a1e2d621eda))
* nodiscard (silence warnings) ([69421c1](https://github.com/GenSpectrum/LAPIS-SILO/commit/69421c158424dd75fba9c3804ebf6d0d89262aea)), closes [#390](https://github.com/GenSpectrum/LAPIS-SILO/issues/390)
* non default unaligned nucleotide sequence prefix ([93b4829](https://github.com/GenSpectrum/LAPIS-SILO/commit/93b4829d834269cfbbc86fb1eb53a29bb0659190))
* nucleotide symbol equals with dot ([6ad623e](https://github.com/GenSpectrum/LAPIS-SILO/commit/6ad623eeaf9089808bdfc29c9b30d1486f2f9782))
* only apply order-by if the field is set, validate orderBy fields for all operations ([405a7f1](https://github.com/GenSpectrum/LAPIS-SILO/commit/405a7f1657c94c327fc667eabcef14e07e858779))
* pango lineage filter with null values ([b7238a8](https://github.com/GenSpectrum/LAPIS-SILO/commit/b7238a8ddced5c2661d8778b838c3a5aa0bd860c))
* parse error messages for mutation filter expressions ([9e4612d](https://github.com/GenSpectrum/LAPIS-SILO/commit/9e4612d34b3e4b50909d3759b1dfc5050783fa6d))
* put compressors into sql function to avoid static variables ([c1a11c8](https://github.com/GenSpectrum/LAPIS-SILO/commit/c1a11c88676d597d4ed66f7611a42a8b726c3753))
* quoting {} in "x.{}" SQL struct accesses, as a string starting with a number leads to parser errors ([#409](https://github.com/GenSpectrum/LAPIS-SILO/issues/409)) ([f3ba6db](https://github.com/GenSpectrum/LAPIS-SILO/commit/f3ba6dbfbaf276ee7161d79751bdb68533e2c19a))
* random (but deterministic for a version) result can depend on internal state, which was changed with duckdb update ([9d9351b](https://github.com/GenSpectrum/LAPIS-SILO/commit/9d9351b27353a8bc8630f9498f8e390bb5b4ca56))
* recursive file reading for nodejs&lt;20 ([9ac0ffe](https://github.com/GenSpectrum/LAPIS-SILO/commit/9ac0ffebc5fd7d831fc4b37bb54a0493be31b822))
* remove caching for linter. Docker image to large on github actions. ([8eaeee6](https://github.com/GenSpectrum/LAPIS-SILO/commit/8eaeee6a1395ef10006f2a8ff3044ffef1b6007c))
* response format ([bca4961](https://github.com/GenSpectrum/LAPIS-SILO/commit/bca49612e55245ceff20e26ca37e0e1260912dd0))
* revert duckdb migration due to it being unable to build the new version on the GitHub runner ([2a62c54](https://github.com/GenSpectrum/LAPIS-SILO/commit/2a62c54d8de3aebabf20394c3b58c15c43773f08))
* revert test numbers to pre-optimization ([99b600a](https://github.com/GenSpectrum/LAPIS-SILO/commit/99b600abda10d4d204ebc2f69cae1d2aa5ebeb4e))
* Roaring from 1.3.0 -&gt; 1.0.0 because of broken CI ([ce82c77](https://github.com/GenSpectrum/LAPIS-SILO/commit/ce82c779c453882c2fa0491778c5df2163d1ce48))
* seralization for insertion_index and insertion_column ([ee99f8d](https://github.com/GenSpectrum/LAPIS-SILO/commit/ee99f8d2dd0631b20ff78026f716c6c8a40411ae))
* single partition build fix ([a8af1c9](https://github.com/GenSpectrum/LAPIS-SILO/commit/a8af1c9792b9d95fc9d146e4a12daec72cc7a4fb))
* specify namespace fmt in calls to format_to ([#353](https://github.com/GenSpectrum/LAPIS-SILO/issues/353)) ([62ffa3b](https://github.com/GenSpectrum/LAPIS-SILO/commit/62ffa3b9b3cf9b14a50cfafd87ce10adae2b7294))
* specifying apk versions ([2c2354e](https://github.com/GenSpectrum/LAPIS-SILO/commit/2c2354edea80063b941a5e84f264401bff4a9b1b))
* test cases verifying that the positions index for mutation distribution are now 1-indexed ([fdf972a](https://github.com/GenSpectrum/LAPIS-SILO/commit/fdf972a631ec49e8186a00b089e5b96be1f0617a))
* test with deterministic results, remove 2 unused variables ([96a424b](https://github.com/GenSpectrum/LAPIS-SILO/commit/96a424b2610865203eae460baac7dc2a388f91db))
* unit test info number updates for new pango-lineages in test data ([39d07c2](https://github.com/GenSpectrum/LAPIS-SILO/commit/39d07c2e5e3970a1ad819751834cdb859a9c591c))
* unit tests and mock fixtures ([db506a1](https://github.com/GenSpectrum/LAPIS-SILO/commit/db506a1ff2eb03249f5ba58eb3faa42255a0a27a))
* update cmake version on ubuntu ([227a2dd](https://github.com/GenSpectrum/LAPIS-SILO/commit/227a2dde0f0d8fa3dfeaf4f4fe48c900f54561f2))
* Upper and lower bound should be inclusive in DateBetween filter ([53c6c05](https://github.com/GenSpectrum/LAPIS-SILO/commit/53c6c05f8c7a66b6dbb67d9a9535831d7f0fa21e))
* Wrong compare function used in multi-threaded case, which displayed wrong tuples in the details endpoint when a limit was used ([650bf36](https://github.com/GenSpectrum/LAPIS-SILO/commit/650bf367427e2d92a6e29be4c01438dffa7358f6))
* zstd dependency ([c145722](https://github.com/GenSpectrum/LAPIS-SILO/commit/c1457228bdca3996a58ea4b785aa330caf96802d))
