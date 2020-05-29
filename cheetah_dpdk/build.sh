#!/bin/bash

if [ "$1" == "test" ]; then
  printf "Building tests.."
  gcc cheetah_aggregate_processing.c -o build/cheetah_test
  echo "done."
else
  rm -rf Makefile
  cp Makefile_worker Makefile
  make
  rm -rf Makefile
  cp Makefile_master Makefile
  make
  rm -rf Makefile
fi
