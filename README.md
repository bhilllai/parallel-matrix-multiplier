# parallel-matrix-multiplier
# Parallel Matrix Multiplier

The program is a part of an assignment for the **CS F372 - Operating System** course of **[BITS Pilani, Hyderabad Campus](https://www.bits-pilani.ac.in/hyderabad/)**.  
The objective of the assignment was to multiply arbitrarily large matrices using parallelism provided by the Linux process and threads libraries.

## Problem Statement

1. Write a C program `reader` which takes three filenames (in1.txt, in2.txt and out.txt) as command line arguments. The first two files (in1.txt and in2.txt) contain two matrices of arbitrary size but satisfying the criteria for matrix multiplication. The sizes will be passed in the command line. `reader` spawns n threads which each read row(s) and column(s) each from `in1.txt` and `in2.txt`.
    1. Different threads should read different parts of the file. Vary the number of threads from 1… to arbitrarily large.
    1. Record the time that it takes to read the entire file into memory with different number of threads (1, 2, … n). The timing should be at the granularity of nanoseconds.
    1. Plot time against the number of threads for different input sizes. Analyse the findings from the plots.

2. Write a C program (`multiplier`) which uses IPC mechanisms to receive the rows and columns
read by `reader`. `multiplier` spawns multiple threads to compute the cells in the product matrix. The program stores the product matrix in the file `out.txt`
    1. Vary the number of threads from 1… to arbitrarily large.
    1. Record the time it takes to compute the product with different number of threads. The timing should be at the granularity of nanoseconds.
    1. Plot the time against the number of threads for different input sizes. Analyse the findings from the plots.

3. Write a scheduler program `scheduler`. `scheduler` spawns 2 children processes which exec to become the processes `reader` and `multiplier`. `scheduler` uses some mechanism to simulate a uniprocessor scheduler. That is, it suspends `reader` and lets `multiplier` execute, and vice versa.
Simulate the following scheduling algorithms in `scheduler`:
    1. Round Robin with time quantum 2 ms.
    1. Round Robin with time quantum 1 ms.

## Running the Program

1. Place the input 1 and input 2 txt files in the folder.
2. Run the following in the terminal:

    ```
    gcc -o P1.out -pthread reader.c
    gcc -o P2.out -pthread multiplier.c
    gcc -o S.out -pthread scheduler.c
    ./S.out <I> <J> <K> <in1.txt> <in2.txt> <out.txt>
    ```

3. The matrix product will be printed in the output file name provided.

