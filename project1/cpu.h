/*
    Zach Tang CS 4348.006
*/

#include <unistd.h>
#include <iostream>
#include <functional>
#include <fstream>

constexpr const int READ {0};
constexpr const int WRITE {1};

class CPU {
public:
    using CPUFcnPtr = void (CPU::*)();

    CPU(int cpuReadFds, int mmuWriteFds, int X):
        timer {Timer{X}},
        cpuReadFds{cpuReadFds}, mmuWriteFds{mmuWriteFds}, PC{0}, SP{999},
        intEnabled {true}, intFlag {false}, timFlag{false} {
        // Point cerr to debug file
        debugFile.open( "debugFile.out" );
        std::cerr.rdbuf( debugFile.rdbuf() );
    }

    ~CPU() {
        debugFile.close();
    }

    void FetchDecodeExecute();

private:
    class Timer {
    public:
        // Interrupt every X instruction
        Timer(int X) : X{X}, clock{0} {}
        
        void Tick(CPU &cpu) {
            clock += 1;
            if (clock == X) {
                clock = 0;
                if (!cpu.inKernel(cpu.PC)) {
                    std::cerr << "Clocked Interrupt\n";
                    cpu.timFlag = true;
                }
            }
        }
    private:
        int X;
        int clock;
    };

    // Internal timer for clocking every X instructions
    Timer timer;

    // Piping for communciating with MMU
    int cpuReadFds, mmuWriteFds;

    // CPU Registers
    int PC, SP, IR, AC, X, Y;

    // Interrupt flags
    bool intEnabled;
    bool intFlag;
    bool timFlag;

    // Store cerr contents
    std::ofstream debugFile;

    // Helps determine what kind of memory access we're doing
    bool inKernel(int address) { return address >= 1000 && address <= 1999; }

    // Functions for interfacing with MMU
    int Read(int address) {
        
        if (!inKernel(PC) && inKernel(address)) {
            throw std::runtime_error("Memory violation: accessing system address " + std::to_string(address) + " in user mode");
        }

        int op = READ;
        
        write(mmuWriteFds, &op, sizeof(op));
        write(mmuWriteFds, &address, sizeof(address));

        int val;
        int bytesRead = read(cpuReadFds, &val, sizeof(val));
        
        // std::cerr << "Address: " << address << " Value: " << val << '\n';
        
        return val;
    }

    int ReadNext() {
        int next { Read(PC) };
        PC += 1;
        return next;
    }

    void Write(int address, int val) {
        int op = WRITE;

        write(mmuWriteFds, &op, sizeof(op));
        write(mmuWriteFds, &address, sizeof(address));
        write(mmuWriteFds, &val, sizeof(val));
        
    }

    // Instruction Set
    CPUFcnPtr Decode(int instruction);
    void Load_Value();
    void Load_Addr();
    void LoadInd_Addr();
    void LoadIdxX_Addr();
    void LoadIdxY_Addr();
    void LoadSpX();
    void Store_Addr();
    void Get();
    void Put_Port();
    void AddX();
    void AddY();
    void SubX();
    void SubY();
    void CopyToX();
    void CopyFromX();
    void CopyToY();
    void CopyFromY();
    void CopyToSp();
    void CopyFromSp();
    void Jump_Addr();
    void JumpIfEqual_Addr();
    void JumpIfNotEqual_Addr();
    void Call_Addr();
    void Ret();
    void IncX();
    void DecX();
    void Push();
    void Pop();
    void Int();
    void IRet();
    void End();
};

void CPU::FetchDecodeExecute() {

    timer.Tick(*this);

    // FDE
    IR = ReadNext();

    auto fcn { Decode(IR) };

    (this->*fcn)();

    // Handle Interrupts
    if (intEnabled && (intFlag || timFlag)) {
        intEnabled = false;

        // Save SP and PC on system stack
        Write(1999, SP);
        Write(1998, PC);
        SP = 1997;

        if (timFlag) {
            timFlag = false;
            PC = 1000;
        } else {
            intFlag = false;
            PC = 1500;
        }
    }

}

void CPU::Load_Value() {
    std::cerr << "CPU::Load_Value\n";
    AC = ReadNext();
}

void CPU::Load_Addr() {
    std::cerr << "CPU::Load_Addr\n";

    int address = ReadNext();
    AC = Read(address);
}

void CPU::LoadInd_Addr() {
    std::cerr << "CPU::LoadInd_Addr\n";

    int address = ReadNext();
    AC = Read(Read(address));
}

void CPU::LoadIdxX_Addr() {
    std::cerr << "CPU::LoadIdxX_Addr\n";
    int address = ReadNext();
    AC = Read(address + X);
}

void CPU::LoadIdxY_Addr() {
    std::cerr << "CPU::LoadIdxY_Addr\n";
    int address = ReadNext();
    AC = Read(address + Y);
}

void CPU::LoadSpX() {
    std::cerr << "CPU::LoadSpX\n";
    AC = Read(SP + X + 1);
}

void CPU::Store_Addr() {
    std::cerr << "CPU::Store_Addr\n";
    Write(ReadNext(), AC);
}

void CPU::Get() {
    std::cerr << "CPU::Get\n";

    // FIXME make this random from 1 to 100
    AC = 20;
}

void CPU::Put_Port() {
    std::cerr << "CPU::Put_Port\n";
    int port {ReadNext()};

    switch(port) {
        case 1:
            std::cout << AC;
            break;
        case 2:
            std::cout << static_cast<char>(AC);
            break;
        default:
            std::cout << "Unrecognized port " << port << '\n';
            exit(0);
    }
}

void CPU::AddX() {
    std::cerr << "CPU::AddX\n";
    AC += X;
}

void CPU::AddY() {
    std::cerr << "CPU::AddY\n";
    AC += Y;
}

void CPU::SubX() {
    std::cerr << "CPU::SubX\n";
    AC -= X;
}

void CPU::SubY() {
    std::cerr << "CPU::SubY\n";
    AC -= Y;
}

void CPU::CopyToX() {
    std::cerr << "CPU::CopyToX\n";
    X = AC;
}

void CPU::CopyFromX() {
    std::cerr << "CPU::CopyFromX\n";
    AC = X;
}

void CPU::CopyToY() {
    std::cerr << "CPU::CopyToY\n";
    Y = AC;
}

void CPU::CopyFromY() {
    std::cerr << "CPU::CopyFromY\n";
    AC = Y;
}

void CPU::CopyToSp() {
    std::cerr << "CPU::CopyToSp\n";
    SP = AC;
}

void CPU::CopyFromSp() {
    std::cerr << "CPU::CopyFromSp\n";
    std::cerr << "Write value " << SP << " to AC " << '\n';
    AC = SP;
}

void CPU::Jump_Addr() {
    std::cerr << "CPU::Jump_Addr\n";
    PC = ReadNext();
}

void CPU::JumpIfEqual_Addr() {
    std::cerr << "CPU::JumpIfEqual_Addr\n";
    int address { ReadNext() };
    if (AC == 0) {
        PC = address;
    }
}

void CPU::JumpIfNotEqual_Addr() {
    std::cerr << "CPU::JumpIfNotEqual_Addr\n";
    int address { ReadNext() };
    if (AC != 0) {
        PC = address;
    }
}

void CPU::Call_Addr() {
    std::cerr << "CPU::Call_Addr\n";
    Write(SP, PC + 1);
    SP -= 1;
    PC = ReadNext();
}

void CPU::Ret() {
    std::cerr << "CPU::Ret\n";
    SP += 1;
    PC = Read(SP);
}

void CPU::IncX() {
    std::cerr << "CPU::IncX\n";
    X += 1;
}

void CPU::DecX() {
    std::cerr << "CPU::DecX\n";
    X -= 1;
}

void CPU::Push() {
    std::cerr << "CPU::Push\n";
    std::cerr << "Write value " << AC << " to SP " << SP << '\n';
    Write(SP, AC);
    SP -= 1;
}

void CPU::Pop() {
    std::cerr << "CPU::Pop\n";
    SP += 1;
    AC = Read(SP);
}

void CPU::Int() {
    std::cerr << "CPU::Int\n";

    // To make it easy, don't allow interrupts during sys calls or vice versa
    if (inKernel(PC)) {
        return;
    }

    intFlag = true;
}

void CPU::IRet() {
    std::cerr << "CPU::IRet\n";
    
    intEnabled = true;

    SP += 1;

    // Hold prev SP in temp variable to avoid bad memory access when PC changes to user
    int newSP = Read(SP + 1);
    PC = Read(SP);
    SP = newSP;
}

void CPU::End() {
    std::cerr << "CPU::End\n";
    exit(0);
}


CPU::CPUFcnPtr CPU::Decode(int instruction) {
    // Map instruction code to function
    switch(instruction) {
        case 1:
            return &CPU::Load_Value;
        case 2:
            return &CPU::Load_Addr;
        case 3:
            return &CPU::LoadInd_Addr;
        case 4:
            return &CPU::LoadIdxX_Addr;
        case 5:
            return &CPU::LoadIdxY_Addr;
        case 6:
            return &CPU::LoadSpX;
        case 7:
            return &CPU::Store_Addr;
        case 8:
            return &CPU::Get;
        case 9:
            return &CPU::Put_Port;
        case 10:
            return &CPU::AddX;
        case 11:
            return &CPU::AddY;
        case 12:
            return &CPU::SubX;
        case 13:
            return &CPU::SubY;
        case 14:
            return &CPU::CopyToX;
        case 15:
            return &CPU::CopyFromX;
        case 16:
            return &CPU::CopyToY;
        case 17:
            return &CPU::CopyFromY;
        case 18:
            return &CPU::CopyToSp;
        case 19:
            return &CPU::CopyFromSp;
        case 20:
            return &CPU::Jump_Addr;
        case 21:
            return &CPU::JumpIfEqual_Addr;
        case 22:
            return &CPU::JumpIfNotEqual_Addr;
        case 23:
            return &CPU::Call_Addr;
        case 24:
            return &CPU::Ret;
        case 25:
            return &CPU::IncX;
        case 26:
            return &CPU::DecX;
        case 27:
            return &CPU::Push;
        case 28:
            return &CPU::Pop;
        case 29:
            return &CPU::Int;
        case 30:
            return &CPU::IRet;
        case 50:
            return &CPU::End;
        default:
            throw std::runtime_error( "Instruction " + std::to_string(instruction) + " not recognized");
    }
}
