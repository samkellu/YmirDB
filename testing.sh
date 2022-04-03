#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'

for test in tests/*.in
do
  test_name=$(echo $test | cut -d . -f1)
  echo -e "${NC}Running Test: ${YELLOW}$test_name${NC}"
  touch $test_name.out

  gcc -o ymirdb ymirdb.c -fsanitize=address -g

  ./ymirdb < $test &> $test_name.out

  ./ymirdb < $test &> $test_name.test

  diff $test_name.out $test_name.test && echo -e "${GREEN}Test $test_name passed." || echo -e "${RED}Test $test_name failed."
  rm $test_name.test
done
