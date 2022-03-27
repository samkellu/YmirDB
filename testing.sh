#!/bin/bash

for test in tests/*.in
do
  test_name=$(echo $test | cut -d . -f1)
  echo Running Test: $test_name
  touch $test_name.out

  # compiling for ASAN which I think is what we are meant to do
  gcc -o ymirdb ymirdb.c -fsanitize=address

  # creating .out files from .ins with ASAN kinda cheating tho so u gotta check the output is actually right lol
  ./ymirdb < $test &> $test_name.out

  # for testing in vs out with asan
  diff $test_name.out < (./ymirdb < $test) && echo "Test $test_name passed." || echo "Test $test_name failed."

  # compiling for valgrind testing if u wanna do that
  # gcc -o ymirdb ymirdb.c -g

  # For writing testcase.out files from a given .in file with valgrind
  # valgrind --leak-check=full ./ymirdb < $test &> $test_name.out

  # For testing .in vs .out with valgrind
  # diff $test_name.out < (valgrind --leak-check=full ./ymirdb< $test) && echo "Test $test_name passed." || echo "Test $test_name failed."
done
