# 100K Threads Exercise: Valgrind & Resource Management Analysis

This document outlines the analysis of the thread creation exercise when
executing without `pthread_join`, utilizing Valgrind for memory profiling, and
resolving the resulting resource leaks.

## 1. Valgrind Limitations & OS Intervention

- **Execution:** Attempting to run the creation of 100,000 threads without
  `pthread_join` through Valgrind resulted in an engine abort
  (`Max number of threads is too low`) and OS-level termination (`Killed`).
- **Analysis:** Valgrind profiles programs within a virtual machine that tracks
  memory per thread, strictly limited to 500 concurrent threads by default.
  Executing an uncontrolled loop of 100,000 `pthread_create` calls overwhelms
  this limit. Furthermore, Valgrind's intensive tracking structures consume
  excessive Host RAM, eventually triggering the Linux OOM Killer.

## 2. Timing Gaps & Read Race Conditions

- **Execution:** To bypass Valgrind's thread limit, the thread count was reduced
  to 499. The array validation check returned incomplete results (e.g.,
  `Correct entries: 431`, `First wrong index: 288`).
- **Analysis:** Removing `pthread_join` breaks synchronization between the main
  thread and worker threads. Because Valgrind heavily slows down execution, the
  main thread rapidly reaches the array validation loop before the newly created
  worker threads have had CPU time to wake up and write their indices. This
  demonstrates a non-deterministic read-write race condition that relies entirely
  on the OS scheduler.

## 3. Resource Leaks & Resolution

When executing the threads without `join`, Valgrind reported a memory leak
(`possibly lost: 135,728 bytes in 499 blocks`). This occurs because threads are
created as *joinable* by default, meaning their Thread Local Storage (TLS) and
internal data structures remain allocated until explicitly joined. If the main
process dies prematurely, this memory becomes orphaned.

- **Automatic Resource Cleanup:** Since `pthread_join` was prohibited by the
  task requirements, the resource leak was solved by transitioning threads to a
  **Detached** state. By invoking `pthread_detach(pthread_self())` inside the
  thread routine (`InsertVal`), the OS is instructed to automatically reclaim
  each thread's memory upon its completion, fully resolving Valgrind's reports.

Another solution is to use semaphores.
