#include <iostream>
#include "fileSystem.hpp"
#include <string>
#include <exception>


void menu(const std::string& alloc) {
    std::cout << "-------- " << alloc << " --------\n" <<
                 "1) Display a file\n" <<
                 "2) Display the file table\n" <<
                 "3) Display the free space bitmap\n" <<
                 "4) Display a disk block\n" <<
                 "5) Copy a file from the simulation to a file on the real system\n" <<
                 "6) Copy a file from the real system to a file in the simulation\n" <<
                 "7) Delete a file\n" <<
                 "8) Exit\n";
}

int main() {

    int choice;

    // File system selection
    std::cout << "1) Contiguous\n" <<
                 "2) Chained\n" <<
                 "3) Indexed\n";

    while (std::cout << "Select 1 - 3: " && (!(std::cin >> choice) || (choice < 1 || choice > 3))) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    
    FileSystem* fs = nullptr;
    std::string alloc;

    // Instantiate file system based on choice
    switch (choice) {
        case 1:
            fs = new ContiguousFileSystem();
            alloc = "Contiguous File System";
            break;
        case 2:
            fs = new ChainedFileSystem();
            alloc = "Chained File System";
            break;
        default:
            fs = new IndexedFileSystem();
            alloc = "Indexed File System";
    }

    std::string srcFileName;
    std::string toFileName;

    // File system user interface
    do {
        // Display menu for user
        menu(alloc);
        
        // Accept user choice
        while (std::cout << "Select 1 - 8: " && !(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        
        // Fulfill user choice
        switch (choice) {
            case 1:
                std::cout << "Enter file name: ";
                std::getline(std::cin, srcFileName);
                std::getline(std::cin, srcFileName);
                fs->displayFile(srcFileName);
                break;
            case 2:
                fs->displayFileTable();
                break;
            case 3:
                fs->displayBitmap();
                break;
            case 4:
                int diskBlock;

                while (std::cout << "Enter disk block number (0 - 255): " && (!(std::cin >> diskBlock) || (diskBlock < 0 || diskBlock > 255))) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
                std::cout << "Disk block: " << diskBlock << '\n';

                fs->displayDiskBlock(diskBlock);
                break;
            case 5:
                std::cout << "Copy from: ";
                std::getline(std::cin, srcFileName);
                std::getline(std::cin, srcFileName);

                std::cout << "Copy to: ";
                std::getline(std::cin, toFileName);

                fs->copyToComputer(srcFileName, toFileName);

                break;
            case 6:
                std::cout << "Copy from: ";
                std::getline(std::cin, srcFileName);
                std::getline(std::cin, srcFileName);

                std::cout << "Copy to: ";
                std::getline(std::cin, toFileName);

                fs->copyFromComputer(srcFileName, toFileName);

                break;
            case 7:
                std::cout << "Enter file name to delete in file system: ";
                std::getline(std::cin, srcFileName);
                std::getline(std::cin, srcFileName);
                fs->deleteFile(srcFileName);
                break;
            case 8:
                std::cout << "Exiting...\n";
                break;
        }
    
    } while (choice != 8);

    delete fs;

    return 0;
}