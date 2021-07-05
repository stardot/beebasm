# Testing

This directory contains tests for beebasm.  They require python3.

Run the tests from the directory above using either
`python test/testrunner.py` or `python3 test/testrunner.py`.

# Tests

The test runner scans `test` and any subdirectories for files with a
`.6502` extension.  Subdirectories are scanned in alphabetical order to
allow simpler tests to be prioritised.

It distinguishes between include files (`.inc.6502`), failure tests
(`.fail.6502`) and success tests (`.6502`).  Include files are ignored.
Failure and success tests are assembled with beebasm.

The first line of a `.6502` file can be a comment with extra
command-line options to pass to beebasm.  For example:

```
\ beebasm -do test.ssd
```

The `-v` and `-i` options are always set by the test runner.

If a test file has a corresponding `.gold.ssd` file this is assumed to be
known-good output from running the test.  The test runner will add the
`-do` option to the command-line.  For success tests, if will also check
that the `.ssd` produced by the test is identical to the gold ssd.

For example, if a directory contains `test.6502` and `test.gold.ssd` then
the test will be required to produce a `test.ssd` file that is identical
to `test.gold.ssd`.

