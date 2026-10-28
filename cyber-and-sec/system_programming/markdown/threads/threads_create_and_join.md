# 100K Threads Exercise: Execution & Performance Analysis

This document outlines the implementation and results of the 100,000 threads
creation exercise, adhering to the specified requirements.

## 1. Thread Creation & Race Condition Prevention

- **Implementation:** The program successfully creates 100,000 threads using
  `pthread_create`.
- **Race Condition Avoidance:** To ensure thread safety without locks, the loop
  index `i` is passed directly to the thread routine by casting it by value
  (`(void*)(size_t)i`). Inside the routine, each thread retrieves its unique
  index and writes exclusively to `arr_g[my_index]`. This guarantees no two
  threads attempt to write to the same memory location simultaneously.

## 2. Array Validation

- **Implementation:** After waiting for all threads to finish using
  `pthread_join`, the main thread iterates through the global array to verify
  data integrity.
- **Result:** The program successfully output `Correct entries: 100000`,
  confirming that every single thread successfully wrote its assigned value to
  the correct index without data corruption.

## 3. Performance Measurement & Optimization

The performance was measured using the Linux `time` command under two different
compilation settings.

**Debug Compilation (No Optimization):**

- `real`: 3.228s
- `user`: 0.379s
- `sys`: 4.432s

**Release Compilation (Optimized with `-O3`):**

- `real`: 2.859s
- `user`: 0.400s
- `sys`: 4.224s

### Conclusion

Applying the `-O3` optimization flag provided a minor improvement in overall
execution time (`real` time dropped from ~3.2s to ~2.8s). The overwhelming
majority of processing time is spent in Kernel space (`sys` > 4.2s), while User
space (`user` ~ 0.4s) takes a fraction of a second. This demonstrates that the
primary bottleneck is OS-level overhead required to allocate, context-switch,
and destroy 100,000 threads — an area where compiler optimizations have minimal
impact.
