#!/bin/sh
lcov -c -d CMakeFiles -o cov.info
genhtml cov.info -o out --legend
open out/index.html
