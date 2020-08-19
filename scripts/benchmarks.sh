#!/bin/bash

# Assume that project was built in directory `ROOT_DIR/build`.

benchmark_names=(
  "benchmark_sequential_skip_list"
)

run_benchmarks() {
  cd build/benchmarks
  for benchmark in "${benchmark_names[@]}"; do
    chmod +x "$benchmark" && ./"$benchmark"
  done
}

(run_benchmarks)
