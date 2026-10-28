# Multithreaded Workload Exercise: Sum of Divisors & Performance Optimization

This document outlines the implementation, system limit adaptation, and
architectural optimization of distributing a heavy computational workload across
multiple threads.

## 1. System Limits & Thread Management

- **System Capacity Analysis:** Using `ulimit -u`, the maximum number of
  concurrent user processes/threads was identified (capped at 55,395).
- **Thread Pool Sizing:** To prevent OS starvation and avoid massive
  context-switching overhead, `NUM_OF_THREADS` was strategically set to
  `10,000` — a safe margin well below the system's hard limit.
- **Synchronization:** The program strictly utilizes `pthread_join`. The main
  thread suspends execution and waits for all 10,000 threads to complete before
  proceeding.

## 2. Distributed Workload & The Algorithm

- **The Target:** The computational workload is executed against a 10-digit
  positive integer (`1,000,000,000`).
- **Range Division:** A `RANGE_PER_THREAD` constant (`100,000`) was introduced.
  A dedicated `range_t` structure is dynamically allocated for each worker,
  granting it a completely isolated segment (Thread 0: 1 to 100,000; Thread 1:
  100,001 to 200,000; etc.).
- **Local Execution:** Each thread independently iterates through its assigned
  range, applies the modulo operator to determine valid divisors, and accumulates
  a local sum.

## 3. Data Integrity & Race Condition Prevention

- **Preventing Integer Overflow:** Accumulator variables were upgraded from
  32-bit `int` to 64-bit `size_t` to safely store the massive sum.
- **Memory Isolation:** Passing a unique memory address (`&ranges[i]`) to each
  thread completely eliminates non-deterministic read/write race conditions.
- **Main Thread Aggregation:** Worker threads do not update a shared global
  variable concurrently. The main thread safely aggregates `sum_of_divisors`
  from all isolated `range_t` structures *after* all threads have been joined.

## 4. Time Performance Analysis

Profiling with the Linux `time` command:

- `real`: ~1.210s
- `user`: ~6.421s
- `sys`: ~0.615s

**Conclusion:** Adapting the thread count to the system's limits resulted in an
exponential performance boost. The `sys` time plummeted to ~0.6s, proving that
severe OS thread overhead (stack allocations, context-switching) was fully
mitigated. The `real` wall-clock time of ~1.2s being significantly lower than
combined processing time (`user` + `sys`) demonstrates highly optimized,
parallel utilization of the CPU's multiple logical cores.
