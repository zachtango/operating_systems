#ifndef FILETABLE_HPP

#define FILETABLE_HPP

#include <string>
#include <unordered_map>
#include <tuple>
#include <iostream>
#include <iomanip>


class FileTable {
public:
    void display() {
        // Print title
        std::cout << ' ';
        for (int i = 0; i < 8; i++) {
            std::cout << "-";
        }
        std::cout << "FILE TABLE";
        for (int i = 0; i < 8; i++) {
            std::cout << "-";
        }
        std::cout << '\n';
        
        // Print body
        std::cout << "| " << std::left << std::setw(9) << "Name" << std::setw(8) << "Start" << std::setw(7) << "Length" << " |\n";

        for (auto it : fat) {
            std::cout << "| " << std::left <<
                std::setw(9) << it.first <<
                std::setw(8) << std::get<0>(it.second) <<
                std::setw(7) << std::get<1>(it.second)  << " |\n";
        }
        
        std::cout << ' ';
        for (int i = 0; i < 26; i++) {
            std::cout << "-";
        }
        std::cout << '\n';
    }

    void add(const std::string& name, int block, int length, int size) {
        // Update in memory representation
        fat[name] = {block, length, size};
    }

    void del(const std::string& name) {
        // Update in memory representation
        fat.erase(name);

    }

    std::tuple<int, int, int> get(const std::string& name) {
        // Get values of a file: block index, number of blocks, number of bytes
        return fat.at(name);
    }

    std::vector<std::tuple<int, int, int>> getValues() {
        // Get all values existing in the file table
        std::vector<std::tuple<int, int, int>> res;

        for (auto it = fat.begin(); it != fat.end(); ++it) {
            res.push_back(it->second);
        }
        
        return res;
    }

    int count(const std::string& name) {
        return fat.count(name);
    }

private:

    /*
        Disk representation
        name: array of 5 bits each with 9 elements
        blocks: 8 bits in disk
        length: 4 bits in disk

        In Memory representation
        unordered_map<string, tuple<int, int, int>>
    */
    std::unordered_map<std::string, std::tuple<int, int, int>> fat;

};


#endif


