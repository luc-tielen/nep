# nep

NEP is an abbreviation for "Native Eclair Parser", a parser for the [Eclair
language](https://github.com/luc-tielen/eclair-lang) written in C++.

It is meant as a prototype for a future version written directly in LLVM IR.

## TODO

- parser (implement gaps, fix bugs)
- generator for huge eclair files (e.g. 50MB)
  any language is OK
- C API (luc)
- testsuite
  - use pytest (or lit?)
  - change parser to take filepath as argument (right now it takes none)
  - in python: run parser (with system / subprocess?), each time with different argument, check exit code / tokens array
- later: optimize code even more
- later: port to haskell (with llvm-codegen)
- benchmarks (against 50MB file)
