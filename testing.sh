#!/bin/bash

for test in tests/*.in
do
  test_name=$(echo $test | cut -d . -f1)
  echo Running Test: $test_name
  touch $test_name.out
  gcc -o ymirdb ymirdb.c -fsanitize=address
  ./ymirdb < $test &> $test_name.out
  # diff $test_name.out < (./ymirdb < $test) && echo "Test $test_name passed." || echo "Test $test_name failed."
done
