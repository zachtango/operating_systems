#ifndef FILETABLE_HPP

#define FILETABLE_HPP

#include <string>
#include <unordered_map>
#include <utility>



class FileTable {
public:
    void display() {
        
    }

    void add(const std::string& name, int block, int length) {
        
        // Update disk representation (make disk friend)
        

        // Update in memory representation
        fat[name] = {block, length};
    }

    void del(const std::string& name) {
        if (fat.count(name) == 0) {
            // FIXME throw err
            return;
        }

        // Update disk representation
        

        // Update in memory representation
        fat.erase(name);

    }

private:

    /*
        Disk representation
        name: array of 5 bits each with 9 elements
        blocks: 8 bits in disk
        length: 4 bits in disk

        In Memory representation
        unordered_map<string, pair<int, int>>
    */
    std::unordered_map<std::string, std::pair<int, int>> fat;

};


#endif


