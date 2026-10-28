# Directory Dictionary Attack: Infinity Labs R&D

This document outlines the systematic process of directory enumeration
(brute-forcing) against a target server. We progressed from basic built-in
tools to more specialized, efficient methodologies.

## Phase 1: Basic Enumeration with Default Dictionary

Initially, we used `dirb` with the default `common.txt` wordlist to get an
overview of the server structure.

```bash
dirb https://infinitylabs.co.il/
```

**Result:** This identified standard directories and provided a baseline for
the server's response behavior; most of them were simply refused.

## Phase 2: Targeted Enumeration with Custom Wordlist

To improve results, we used a specialized wordlist from GitHub designed for
more comprehensive testing of common web vulnerabilities and hidden structures.

Wordlist:
`https://raw.githubusercontent.com/dustyfresh/dictionaries/refs/heads/master/DirBuster-Lists/directory-list-2.3-medium.txt`

```bash
dirb https://infinitylabs.co.il/ directory-list-2.3-medium.txt
```

**Result:** This scan revealed deeper, more specific directory structures that
were missed by the standard `common.txt` file.

## Phase 3: Optimized Performance with gobuster

`dirb` is reliable but slower for large wordlists. We switched to `gobuster`,
which is written in Go and significantly faster.

```bash
gobuster dir -u https://infinitylabs.co.il/ -w directory-list-2.3-medium.txt -t 50
```

**Flags:**

- `dir` — directory enumeration mode
- `-u` — the target URL
- `-w` — the path to the wordlist
- `-t 50` — 50 threads for concurrent request processing

**Key addition:** `-x php,html,txt` automatically appends common file
extensions to brute-force attempts.

## Phase 4: Advanced Fuzzing with ffuf

To bypass potential restrictions and discover more specific endpoints, we used
`ffuf` (Fuzz Faster U Fool), which provides more granular control over the
fuzzing process.

```bash
ffuf -w directory-list-2.3-medium.txt -u https://infinitylabs.co.il/FUZZ
```

**Why `ffuf` is more efficient here:**

- **Targeted Fuzzing:** The `FUZZ` keyword can be placed at any point in the
  URL (path, parameter, etc.).
- **Custom Filtering:** Results can be filtered by response size (`-fs`),
  status code (`-fc`), or line count, which drastically reduces noise.

## Conclusion

The combination of directory enumeration (`gobuster`) and targeted URL fuzzing
(`ffuf`) provides a robust strategy for mapping the target. Always ensure the
wordlist is appropriate for the target environment (e.g., use PHP-specific
lists for PHP-based applications).
