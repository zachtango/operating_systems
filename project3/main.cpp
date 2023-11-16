#include <iostream>
#include "fileSystem.hpp"
#include <string>

void menu() {
    std::cout << "1) Display a file\n" <<
                 "2) Display the file table\n" <<
                 "3) Display the free space bitmap\n" <<
                 "4) Display a disk block\n" <<
                 "5) Copy a file from the simulation to a file on the real system\n" <<
                 "6) Copy a file from the real system to a file in the simulation\n" <<
                 "7) Delete a file\n" <<
                 "8) Exit\n";
}

int main() {

    ContiguousFileSystem fs;

    int choice;
    std::string srcFileName;
    std::string toFileName;

    do {
        menu();
        
        std::cin >> choice;

        switch (choice) {
            case 1:
                std::cout << "Enter file name: ";
                std::cin >> srcFileName;
                fs.displayFile(srcFileName);
                break;
            case 2:
                fs.displayFileTable();
                break;
            case 3:
                fs.displayBitmap();
                break;
            case 4:
                int diskBlock;
                std::cout << "Enter disk block number (0 - 255): ";
                std::cin >> diskBlock;
                fs.displayDiskBlock(diskBlock);
                break;
            case 5:
                std::cout << "Enter file name to copy from working directory: ";
                std::cin >> srcFileName;

                std::cout << "Enter file name to store copy as in file system: ";
                std::cin >> toFileName;

                fs.copyFromComputer(srcFileName, toFileName);
                break;
            case 6:
                std::cout << "Enter file name to copy from file system: ";
                std::cin >> srcFileName;

                std::cout << "Enter file name to copy as in working directory: ";
                std::cin >> toFileName;

                fs.copyToComputer(srcFileName, toFileName);
                break;
            case 7:
                std::cout << "Enter file name to delete in file system: ";
                std::cin >> srcFileName;
                
                fs.deleteFile(srcFileName);
                break;
            case 8:
                break;
                break;
            default:
                std::cout << "Enter a number between 1 - 8: ";
        }
    

    } while (choice != 8);


    return 0;
}