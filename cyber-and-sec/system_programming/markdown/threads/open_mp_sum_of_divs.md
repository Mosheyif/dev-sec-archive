# OpenMP Multithreading & Performance Analysis: Sum of Divisors

This document outlines the implementation, compilation, and performance
profiling of the sum of divisors algorithm using the OpenMP API, compared
against manual `pthreads` management.

## 1. OpenMP Implementation

- **Directive-Based Parallelism:** Instead of explicitly managing thread arrays,
  memory allocation, and manual loop partitioning via `pthreads`, the sequential
  baseline loop was parallelized using the OpenMP API.
- **The Parallel For Construct:** The
  `#pragma omp parallel for reduction(+:round_sum)` directive instructs the
  compiler to automatically distribute the iteration space across multiple
  hardware threads.
- **Race Condition Prevention:** The `reduction(+:round_sum)` clause ensures
  that each thread maintains a private accumulation variable during execution,
  safely combining them into `round_sum` upon completion.
- **Workload Amplification:** To satisfy benchmarking constraints and avoid
  system noise, the computational core runs across `10` rounds, averaging the
  final result.

## 2. Compilation & Execution

```bash
gcc open_mp_sum_of_divs.c -fopenmp
```

## 3. Performance Profiling via `perf stat`

Profiling the OpenMP implementation using `perf stat ./a.out`:

- **Elapsed Time (`time elapsed`):** ~8.525 seconds
- **User CPU Time (`user`):** ~65.111 seconds
- **System Overhead (`sys`):** ~0.017 seconds

## 4. Performance Comparison: OpenMP vs. Manual `pthreads`

- **Code Simplicity:** OpenMP drastically reduces boilerplate code by
  eliminating manual thread creation (`pthread_create`), joining
  (`pthread_join`), and explicit boundary calculations (`range_t` structures).
- **Performance Parity:** The execution time (~8.4s) and user CPU consumption
  (~65s) match the optimal multi-threaded performance achieved previously with
  manual `pthreads` (4-8 threads). This demonstrates that OpenMP's runtime
  scheduler implicitly maps threads to hardware cores with high efficiency.
