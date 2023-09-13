# Project 1 Zach Tang CS 4348.006

# Files
- main.cpp
- cpu.h
- mmu.h

### main.cpp
- This is where the CPU and MMU are instantiated to work together. I configure the CPU and
    MMU to run in different processes and communicate using IPC. I set up piping between
    the CPU and MMU in this file.

### cpu.h
- This is the CPU. It reads instructions from memory by interfacing with the MMU. It also
    contains a timer that clocks every X instructions. The timer interrupts and also the user
    code can interrupt as well, which the CPU will handle by switching to the system stack.

### mmu.h
- This is the memory management unit. It loads the program files into an array of integers
    and allows reads and writes to that array of integers (i.e. "memory") by the CPU.

### Compiling and Running
1. To compile, run `g++ -std=c++11 main.cpp`
2. This will produce an `a.out` binary that you can run on the machine
3. The usage of `a.out` is `./a.out <program.txt> <X timer instructions>` where `<program.txt>`
    is this name of the program file holding your instructions and `<X timer instructions>` is
    the rate at which the clock ticks (this must be greater than 1).
4. Example run: `./a.out sample2.txt 4`
    Output:
                ------
             /         \
            /   -*  -*  \
            |           |
            \   \____/  /
             \         /
                ------
