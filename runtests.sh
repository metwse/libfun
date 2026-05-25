#!/bin/bash

make -j -C tests

for test in $(ls ./dist/tests/*.test); do
    valgrind $test > $test.log
done

gcovr *
