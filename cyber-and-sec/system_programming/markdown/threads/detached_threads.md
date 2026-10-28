# 100K Detached Threads Exercise: False Sharing & Synchronization Analysis

This document outlines the implementation, optimization, and architectural
challenges encountered when creating 100,000 detached threads without utilizing
`pthread_join`.

## 1. Detached Threads Implementation

To prevent resource leaks (such as orphaned Thread Local Storage) without using
`pthread_join`, the threads must be configured as **Detached**. This instructs
the OS to automatically reclaim a thread's resources immediately upon its
completion. Both available mechanisms were utilized:

- **Option 1 (Creation Time):** Initializing a `pthread_attr_t` structure,
  setting its state using
  `pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)`, and passing it
  to `pthread_create`.
- **Option 2 (Runtime):** Calling `pthread_detach(pthread_self())` internally
  at the very beginning of the thread's execution routine (`InsertVal`).

## 2. The False Sharing Phenomenon

- **The Problem:** Modern CPUs load memory into L1/L2 cache in chunks called
  **Cache Lines** (typically 64 bytes). An array of standard `int` variables
  places adjacent elements tightly packed on the same cache line. When multiple
  threads simultaneously modify their respective independent indices, the CPU is
  forced to constantly invalidate and reload the entire cache line across
  different cores (**Cache Bouncing** / **False Sharing**), severely degrading
  multicore performance despite the code being race-condition-free.
- **The Solution (Memory Padding):** A custom `struct cache_aligned_int_t` was
  introduced containing the target integer (`value`) and a `char` array padded
  to exactly `CACHE_LINE_SIZE - sizeof(int)` bytes. Each thread now operates on
  a completely distinct, isolated cache line, eliminating hardware-level
  conflicts entirely.

## 3. The Main Thread Exit Problem & Synchronization

- **The Problem:** Because threads are detached, the main thread lacks a
  built-in mechanism to track their lifecycle. Two major issues occur:
  1. **Read-Write Race Condition:** The main thread may attempt to validate the
     array before worker threads have had CPU time to write their indices.
  2. **Premature Termination:** If `main` reaches `return 0;`, the OS
     terminates the entire process, killing any mid-execution worker threads.
     Using `pthread_exit(NULL)` prevents process termination but does not solve
     the timing issue.

- **The Semaphore Solution:**
  1. Initialize a Semaphore to `0` and a global atomic counter to `0`.
  2. As each worker thread finishes its assignment, it safely increments the
     global counter.
  3. The *very last* thread to finish (counter hits `NUM_OF_THREADS`) calls
     `sem_post()` to signal the semaphore.
  4. The main thread calls `sem_wait()` before the validation loop. It blocks
     efficiently without wasting CPU cycles. Once the final thread signals,
     the main thread wakes up, validates the fully populated array, and exits
     gracefully.
