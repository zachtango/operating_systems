/*
    Zach Tang CS 4348.006
*/

#include "mmu.h"
#include "cpu.h"
#include <unistd.h>
#include <signal.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cout << "Usage: ./a.out <program.txt> <X timer instructions>\n";
        return 1;
    }

    // Clock tick rate given by user
    int X { atoi(argv[2]) };
    if (X <= 1) {
        std::cout << "<X timer instructions> must be greater than 1\n";
        return 1;
    }

    // Piping for IPC between MMU and CPU
    int mmuPipeFds[2];
    int cpuPipeFds[2];

    if (pipe(mmuPipeFds) == -1 || pipe(cpuPipeFds) == -1) {
        std::cout << "Unable to create pipe(s)\n";
        return 1;
    }

    // Create a parallel process with fork() to run the CPU on
    pid_t child_pid {fork()};

    if (child_pid == 0) {
        // Pipe cleanup (CPU will never use these pipes)
        close(cpuPipeFds[1]);
        close(mmuPipeFds[0]);

        // Run the CPU
        CPU cpu {cpuPipeFds[0], mmuPipeFds[1], X};

        // The CPU will call exit() so this loop will terminate
        while (true) {
            cpu.FetchDecodeExecute();
        }

        return 0;
    }

    // Pipe cleanup (MMU will never use these pipes)
    close(cpuPipeFds[0]);
    close(mmuPipeFds[1]);

    // Load program into memory
    MMU memory;

    if (!memory.LoadProgram(argv[1])) {
        kill(child_pid, SIGKILL);
        return 1;
    }

    while (true) {
        int op;
        int bytesRead = read(mmuPipeFds[0], &op, sizeof(op));

        // Process reads and writes to memory
        if (bytesRead > 0) {
            switch(op) {
                int address;
                int val;
                case READ:
                    read(mmuPipeFds[0], &address, sizeof(address));
                    val = memory.Read(address);
                    write(cpuPipeFds[1], &val, sizeof(val));
                    break;
                case WRITE:
                    read(mmuPipeFds[0], &address, sizeof(address));
                    read(mmuPipeFds[0], &val, sizeof(val));
                    memory.Write(address, val);
                    break;
                default:
                    throw std::runtime_error("Unknown memory operation");
            }
        }

        // If CPU stops execution, then program is done
        int status;
        if (waitpid(child_pid, &status, WNOHANG) > 0) {
            memory.DumpMemory("dump.out");
            break;
        }
    }

    return 0;
}

