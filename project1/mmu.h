/*
    Zach Tang CS 4348.006
*/

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>


class MMU {
public:
    MMU() {
        // Set all addresses to -1 originally to make the memory dump look prettier
        for (int i = 0; i < numAddresses; i++) {
            M[i] = -1;
        }
    }

    // Load instructions into memory array
    bool LoadProgram(std::string fileName);
    
    // Interfacing functions for public to read and write from and to memory
    int Read(int address) { return M[address]; };
    void Write(int address, int data) { M[address] = data; };

    // Dumps memory array to given file
    void DumpMemory(std::string fileName);
private:
    // Memory array
    constexpr static const int numAddresses {2000};
    int M[numAddresses];

};


bool MMU::LoadProgram(std::string fileName) {
    std::ifstream fin;
    
    fin.open(fileName);
        
    if (fin.fail()) {
        std::cout << "Unable to open file " << fileName << '\n';  
        return false; 
    }

    int address = 0;
    std::string s;
    while (std::getline(fin, s)) {
        // Empty line, ignore
        if (s.empty() || (s[0] != '.' && !isdigit(s[0]))) {
            continue;
        }

        // Use string stream to read in number from a string and ignore rest after
        std::istringstream ss(s);
        int v;

        // Period indicates we will change the address
        if (s[0] == '.') {
            ss.seekg(1, std::ios::cur);
            ss >> v;
            address = v;
            continue;
        }

        // Otherwise put number in memory
        ss >> v;
        M[address] = v;
        address += 1;
    }

    fin.close();

    return true;
}

void MMU::DumpMemory(std::string fileName) {
    std::ofstream fout;
    
    fout.open(fileName);

    if (fout.fail()) {
        std::cout << "Unable to open file " << fileName << '\n';
        return;
    }

    for (int i = 0; i < numAddresses; i++) {
        fout << std::setw(4) << i << ' ' << std::setw(10) << M[i] << '\n';
    }

    fout.close();
}

