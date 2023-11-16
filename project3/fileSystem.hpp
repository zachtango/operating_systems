
#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include "disk.hpp"
#include "bitmap.hpp"
#include "fileTable.hpp"
#include "types.hpp"
#include <string>
#include <algorithm>
#include <fstream>
#include <vector>
#include <tuple>
#include <iostream>

class FileSystem {

public:
    void displayFile(const std::string& name) {
        /*
            what to display?? it's name and size?? what do w contents lol
        */
    };

    void displayFileTable() {
        fat.display();
    }

    void displayBitmap() {
        bitmap.display();
    };

    void displayDiskBlock(int block) {
        // Get block from disk

        // Display block ???
    };

    void copyToComputer(const std::string& name, const std::string& dest) {
        // get file contents from disk
        std::vector<char> buffer = getFileData(name);

        // write file to real computer
        std::ofstream fout(dest, std::ios::binary);

        fout.write(buffer.data(), buffer.size());

    }

    void copyFromComputer(const std::string& src, const std::string& name) {
        // get file contents from computer
        std::ifstream fin(src, std::ios::binary | std::ios::ate);

        std::streamsize size = fin.tellg();
        fin.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        if (fin.read(buffer.data(), size)) {
            allocateFile(name, size, buffer);
        }
    }
    
    virtual void deleteFile(const std::string& name) = 0;

protected:
    virtual void allocateFile(const std::string& name, int bytes, const std::vector<char>& buffer) = 0;
    virtual std::vector<char> getFileData(const std::string& name) = 0;


    Disk disk;
    Bitmap bitmap;
    FileTable fat;

};

class ContiguousFileSystem : public FileSystem {
public:
    void deleteFile(const std::string& name) override {

        std::tuple<int, int, int> t = fat.get(name);

        bitmap.deallocate(std::get<0>(t), std::get<1>(t));
        
        fat.del(name);
    }

private:
    void allocateFile(const std::string& name, int bytes, const std::vector<char>& buffer) override {
        int numBlocks = (bytes + NUM_BYTES_PER_BLOCK - 1) / NUM_BYTES_PER_BLOCK;

        int l = bitmap.firstFit(numBlocks);

        if (l == -1) {
            compact();
        }

        l = bitmap.firstFit(numBlocks);

        if (l == -1) {
            // FIXME not enuf space err
            return;
        }

        bitmap.allocate(l, numBlocks);
        fat.add(name, l, numBlocks, buffer.size());

        for (int i = 0; i < numBlocks; i++) {
            Block block {buffer, i * NUM_BYTES_PER_BLOCK, std::min(bytes - (i * NUM_BYTES_PER_BLOCK), NUM_BYTES_PER_BLOCK)};
            disk.assignBlock(block, l + i);
        }
    }

    void compact() {

    }

    std::vector<char> getFileData(const std::string& name) override {
        std::tuple<int, int, int> t = fat.get(name);

        std::vector<char> buffer;

        int numBytes = std::get<2>(t);
        for (int i = 0; i < std::get<1>(t); i++) {
            Block block = disk.getBlock(std::get<0>(t));

            block.copy(buffer, std::min(numBytes, NUM_BYTES_PER_BLOCK));

            numBytes -= NUM_BYTES_PER_BLOCK;
        }

        return buffer;
    }

};

class ChainedFileSystem : public FileSystem {

    void allocateFile(const std::string& name, int bytes, const std::vector<char>& buffer) override {
        int numBlocks = (bytes + NUM_BYTES_PER_BLOCK - 1) / NUM_BYTES_PER_BLOCK;

        int s = bitmap.firstFit(1);
        for (int i = 0; i < numBlocks; i++) {
            int l = bitmap.firstFit(1);

            // allocate in disk too

            bitmap.allocate(l, 1);

        }

        fat.add(name, s, numBlocks, buffer.size());
        
    }
    
    void deleteFile(const std::string& name) {
        // get index from fat and length

        // for num blocks, iterate through pointer and delete in disk and bitmap and ptr
        

    }

    std::vector<char> getFileData(const std::string& name) override {

    }

    void consolidate() {


    }
    

};

class IndexedFileSystem : public FileSystem {
    
    void allocateFile(const std::string& name, int bytes, const std::vector<char>& buffer) override {
        int numBlocks = (bytes + NUM_BYTES_PER_BLOCK - 1) / NUM_BYTES_PER_BLOCK;

        // Entry block
        int s = bitmap.firstFit(1);
        bitmap.allocate(s, 1);

        for (int i = 0; i < numBlocks; i++) {
            // track blocks allocated as a list and store in the index block (entry block)

        }

        fat.add(name, s, numBlocks + 1, buffer.size());
    }

    void deleteFile(const std::string& name) {
        // get index from fat and length

        // go to entry block

        // iterate through block numbers in entry block and delete them on disk and bitmap
    }

};


#endif


