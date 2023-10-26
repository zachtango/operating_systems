
# Project 1 Zach Tang CS 4348.006

# Files
- main.cpp
- Makefile

### main.cpp
- This contains all the code for the hotel simulation, including the guest, front desk employee, and bellhop employee 
    threads. The functions existing in this file match the pseudocode given in the design.

### test.py and test folder
- DISCLAIMER: test.py does not work on the UTD CS servers because python is of version 2.7.5 while test.py was made for python 3.11.4.
    Also, it may error out and say that too many files are open; in this case, run `ulimit -n 2048` to allow up to 2048 files to be open.
- This is not part of the project, but I made a test script to confirm my simulation works correctly. It simulates 1250
    runs (50 for guests between 1 and 25) and stores the output in txt files in the test folder. Then it validates the
    output of those files. Again, this is not part of the actual project, just a tool I thought would be helpful for me.

### Compiling and Running
1. To compile, run `make`
2. This will produce an `hotel` binary that you can run on the machine
3. The usage of `hotel` is `./hotel <number of guests>` where `<number of guests>`
    is the number of guests in the hotel simulation. This must be a number between 1 and 25.
4. Example run: `./hotel 1`
    Output:
        Simulation starts
        Front desk employee 0 created
        Front desk employee 1 created
        Bellhop 0 created
        Bellhop 1 created
        Guest 0 created
        Guest 0 enters hotel with 0 bags
        Front desk employee 0 registers guest 0 and assigns room 1
        Guest 0 receives room key for room 1 from front desk employee 0
        Guest 0 enters room 1
        Guest 0 retires for the evening
        Guest 0 joined
        Simulation ends





