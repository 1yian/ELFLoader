
# ELF Loader

This project, developed for my graduate systems class is the implementation of programs that loads and executes executables in the user-space.

## Motivation

A user-level executable loader has a lot of real-world uses. Namely it provides:

- **Obfuscation**: Enhances security by making it more difficult for unauthorized users to reverse engineer or understand the loading process.
- **Flexibility**: Allows for custom loading strategies, such as more efficient memory management, tailored to specific application needs.
- **Isolation**: Ensures a level of isolation from the system, potentially increasing the safety and stability of the overall system.


## Build

Use the `make` command to build the project:

```bash
make
```

This command builds three components: `apager`, `dpager`, and `hpager`, each differing in their approach to managing memory for the executable.

## Usage

The pagers can be executed as follows:

```bash
./[apager/dpager/hpager] <program executable>
```

## How it Works

The pagers read the ELF header of a program and load the necessary segments and sections of the executable, then jump to the entry point for each program. They differ in their paging policies:

- `apager` implements All-at-once loading, mapping all parts of the executable into memory.
- `dpager` implements Demand-based loading, initially mapping only minimal pages, and loading others as needed.
- `hpager` (similar to `dpager`) uses a Hybrid policy, loading pages on demand and also pre-emptively loading subsequent pages.

All pagers set up a stack in the user-space for the program, containing `argc`, `argv`, `envp`, and `auxv`, and then jump to the program's entry point.

## Tests and Results

To showcase the time and space trade-offs of the different paging policies, I created simple tests. We test apager, dpager, and hpager2/hpager3. 
hpager2 allocates 2 pages extra for each demanded page, while hpager3 allocates 3 pages extra for each demanded page.

All tests involve a very large statically allocated, uninitialized array.

`single_access`: Set the first element of the array.

`seq_access`: Set every element of the array.

`sparse_access`: Set every Nth element of the array, where N is a large number (large number <= N < array size)

`sparse_seq_access`: Set the next M elements starting from every Nth element of the array, where (large number <= M < N < array size)

To run these experiments:

```bash
bash run_experiments.sh
```


The following table shows the mean time and memory usage over 10 trials for each test:

| Test                | Pager   | Mean Time (s) | Mean Memory (KB) |
|---------------------|---------|---------------|------------------|
| single_access       | apager  | 0.030 ± 0.002 | 392975 ± 35      |
|                     | dpager  | 0.000 ± 0.000 | 1642 ± 70        |
|                     | hpager2 | 0.000 ± 0.000 | 1662 ± 45        |
|                     | hpager3 | 0.000 ± 0.000 | 1771 ± 61        |
| seq_access          | apager  | 0.177 ± 0.019 | 392966 ± 44      |
|                     | dpager  | 0.325 ± 0.032 | 392264 ± 48      |
|                     | hpager2 | 0.306 ± 0.013 | 392271 ± 46      |
|                     | hpager3 | 0.297 ± 0.015 | 392368 ± 41      |
| sparse_access       | apager  | 0.024 ± 0.005 | 392955 ± 43      |
|                     | dpager  | 0.040 ± 0.000 | 81661 ± 52       |
|                     | hpager2 | 0.050 ± 0.000 | 121672 ± 40      |
|                     | hpager3 | 0.061 ± 0.003 | 161737 ± 45      |
| sparse_seq_access   | apager  | 0.102 ± 0.006 | 392982 ± 41      |
|                     | dpager  | 0.150 ± 0.000 | 169928 ± 57      |
|                     | hpager2 | 0.141 ± 0.003 | 169695 ± 50      |
|                     | hpager3 | 0.139 ± 0.003 | 178261 ± 43      |



## Analysis
These tests highlight trade-offs in terms of segfault handling and pre-emptive page loading.

In single access tests, `apager` was slower and used more memory, as it loads a large array even when accessing only one element. Demand pagers, by contrast, excelled due to their minimal initial load. Sequential access tests showed `apager` as fastest due to its absence of segfault handling overhead, despite similar memory usage across all pagers. In sparse access tests, `dpager` showed the most efficient memory mapping because it always allocates the exact number of pages necessary for execution, while `apager` had the lowest execution time as it over-allocates pages at the beginning. In sparse-sequential tests, the `hpager`s perform the best because they benefit from locality in the sequential accesses.


## Tech Stack
- C
- x86 Assembly
- Bash (executing tests)
- Python (processing results)
