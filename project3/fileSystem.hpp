
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
        try {
            // Get file data from file system
            std::vector<char> buffer = getFileData(name);

            // Open file on computer to write to
            std::ofstream fout(dest, std::ios::binary);

            if (!fout.is_open()) {
                std::cout << "Unable to open file \"" << dest << "\"\n";
                return;
            }

            // Write file data to file on comptuer
            fout.write(buffer.data(), buffer.size());
        } catch (const std::out_of_range& e) {
            std::cout << "File System Error: \"" << name << "\" does not exist in file system\n";
        }
    }

    void copyFromComputer(const std::string& src, const std::string& name) {
        // Validate file name (only a-z allowed)
        bool validName = name.size() <= 8;
        for (auto &c : name) {
            if (c < 'a' || c > 'z') {
                validName = false;
                break;
            }
        }
        
        if (!validName) {
            std::cout << "File System Error: Invalid file name \"" << name << "\"\n";
            return;
        }

        if (fat.count(name)) {
            deleteFile(name);
        }

        // Open file on computer
        std::ifstream fin(src, std::ios::binary | std::ios::ate);

        if (!fin.is_open()) {
            std::cout << "File System Error: Unable to open file \"" << src << "\"\n";
            return;
        }

        // Get file size
        std::streamsize size = fin.tellg();
        fin.seekg(0, std::ios::beg);

        // Buffer to hold file data
        std::vector<char> buffer(size);
        if (fin.read(buffer.data(), size)) {
            // Allocate file in file system
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
        try {
            // Get block start and length
            std::tuple<int, int, int> t = fat.get(name);

            // Deallocate memory on bitmap
            bitmap.deallocate(std::get<0>(t), std::get<1>(t));
            
            // Remove entry for file allocation table
            fat.del(name);
        } catch (const std::out_of_range& e) {
            std::cout << "File system error: \"" << name << "\" does not exist in file system\n";
        }
    }

private:
    void allocateFile(const std::string& name, int bytes, const std::vector<char>& buffer) override {
        // File size in blocks
        int numBlocks = (bytes + NUM_BYTES_PER_BLOCK - 1) / NUM_BYTES_PER_BLOCK;

        // FIXME: Max file size is 10 blocks
        if (numBlocks > 10) {
            std::cout << "File system error: File too big\n";
            return;
        }

        // Check if enough space exists
        int numOpenBlocks = bitmap.numOpenBlocks();
        if (numOpenBlocks < numBlocks) {
            std::cout << "File system error: Not enough space for file\n";
            return;
        }

        // Find first fit for contiguous set of numBlocks
        int l = bitmap.firstFit(numBlocks);
        
        // Not able to find space for a contiguous set --> compact the memory
        if (l == -1) {
            compact();
            
            // After compaction, first fit should work
            l = bitmap.firstFit(numBlocks);
        }

        // Allocate the contiguous set on the bitmap
        bitmap.allocate(l, numBlocks);

        // Add entry to file allocation table
        fat.add(name, l, numBlocks, buffer.size());

        // Transfer data from buffer to disk
        for (int i = 0; i < numBlocks; i++) {
            Block block {buffer, i * NUM_BYTES_PER_BLOCK, std::min(bytes - (i * NUM_BYTES_PER_BLOCK), NUM_BYTES_PER_BLOCK)};
            disk.assignBlock(block, l + i);
        }
    }

    std::vector<char> getFileData(const std::string& name) override {
        // Get block start, length, and byte length
        std::tuple<int, int, int> t = fat.get(name);

        // Buffer for holding file data
        std::vector<char> buffer;

        // File size in bytes
        int numBytes = std::get<2>(t);

        // Transfer all the blocks of data to the buffer
        for (int i = 0; i < std::get<1>(t); i++) {
            Block block = disk.getBlock(std::get<0>(t));

            block.copy(buffer, std::min(numBytes, NUM_BYTES_PER_BLOCK));

            numBytes -= NUM_BYTES_PER_BLOCK;
        }

        return buffer;
    }

    void compact() {

    }

};

class ChainedFileSystem : public FileSystem {
public:
    void deleteFile(const std::string& name) {
        try {
            // Get block start and length
            std::tuple<int, int, int> t = fat.get(name);

            int blockIndex = std::get<0>(t);
            int numBlocks = std::get<1>(t);
            int numBytes = std::get<2>(t);

            for (int i = 0; i < numBlocks - 1; i++) {
                Block block = disk.getBlock(blockIndex);

                // Deallocate block on bitmap
                bitmap.deallocate(blockIndex, 1);

                // Get next block index
                blockIndex = block.getByte(NUM_BYTES_PER_BLOCK - 1);
            }

            // Deallocate last block on bitmap
            bitmap.deallocate(blockIndex, 1);
            
            // Remove entry for file allocation table
            fat.del(name);
        } catch (const std::out_of_range& e) {
            std::cout << "File system error: \"" << name << "\" does not exist in file system\n";
        }
    }

private:
    void allocateFile(const std::string& name, int bytes, const std::vector<char>& buffer) override {
        // File size in blocks
        int numBlocks = (bytes + NUM_BYTES_PER_BLOCK - 1) / NUM_BYTES_PER_BLOCK;

        // Add the extra byte at the end of each block to account for the pointer used in chained allocation
        numBlocks = (bytes + numBlocks - 1 + NUM_BYTES_PER_BLOCK - 1) / NUM_BYTES_PER_BLOCK;
        // FIXME: Max file size is 10 blocks
        if (numBlocks > 11) {
            std::cout << "File system error: File too big\n";
            return;
        }

        // Check if enough space exists
        int numOpenBlocks = bitmap.numOpenBlocks();
        if (numOpenBlocks < numBlocks) {
            std::cout << "File system error: Not enough space for file\n";
            return;
        }
        
        // Find starting block for first fit
        int curr = bitmap.firstFit(1);
        int next = curr;

        // Add file to file allocation table
        fat.add(name, curr, numBlocks, buffer.size());

        for (int i = 0; i < numBlocks; i++) {
            Block block {buffer, i * NUM_BYTES_PER_BLOCK - i, std::min(bytes, NUM_BYTES_PER_BLOCK - 1)};
            
            // Allocate in bitmap
            bitmap.allocate(curr, 1);
            
            // Set pointer to next block
            next = bitmap.firstFit(1);
            block.setByte(next, NUM_BYTES_PER_BLOCK - 1);
            
            // Allocate on disk
            disk.assignBlock(block, curr);

            bytes -= (NUM_BYTES_PER_BLOCK - 1);
            curr = next;
        }


    }

    std::vector<char> getFileData(const std::string& name) override {
        // Buffer for holding file data
        std::vector<char> buffer;

        // Get block start and length
        std::tuple<int, int, int> t = fat.get(name);

        int blockIndex = std::get<0>(t);
        int numBlocks = std::get<1>(t);
        int numBytes = std::get<2>(t);

        // Transfer blocks of data to buffer
        for (int i = 0; i < numBlocks - 1; i++) {
            Block block = disk.getBlock(blockIndex);

            block.copy(buffer, NUM_BYTES_PER_BLOCK - 1);

            // Get next block index
            blockIndex = block.getByte(NUM_BYTES_PER_BLOCK - 1);

            numBytes -= (NUM_BYTES_PER_BLOCK - 1);
        }

        // Transfer last block of data to buffer
        Block block = disk.getBlock(blockIndex);
        block.copy(buffer, std::min(numBytes, NUM_BYTES_PER_BLOCK));

        return buffer;
    }

};

class IndexedFileSystem : public FileSystem {
public:
    void deleteFile(const std::string& name) {
        try {
            // Get block start and length
            std::tuple<int, int, int> t = fat.get(name);

            int blockIndex = std::get<0>(t);
            int numBlocks = std::get<1>(t);
            int numBytes = std::get<2>(t);

            // Entry block
            Block block = disk.getBlock(blockIndex);
            bitmap.deallocate(blockIndex, 1);

            // Deallocate blocks on bitmap
            for (int i = 0; i < numBlocks - 1; i++) {
                blockIndex = block.getByte(i);
                bitmap.deallocate(blockIndex, 1);
            }

            // Remove entry for file allocation table
            fat.del(name);
        } catch (const std::out_of_range& e) {
            std::cout << "File system error: \"" << name << "\" does not exist in file system\n";
        }
    }


private:
    void allocateFile(const std::string& name, int bytes, const std::vector<char>& buffer) override {
        // File size in blocks (1 extra block to hold index table)
        int numBlocks = (bytes + NUM_BYTES_PER_BLOCK - 1) / NUM_BYTES_PER_BLOCK + 1;

        // FIXME: Max file size is 10 blocks
        if (numBlocks > 10) {
            std::cout << "File system error: File too big\n";
            return;
        }

        // Check if enough space exists
        int numOpenBlocks = bitmap.numOpenBlocks();
        if (numOpenBlocks < numBlocks) {
            std::cout << "File system error: Not enough space for file\n";
            return;
        }

        // Hold block indexes
        std::vector<int> blockIndexes;

        // Entry block
        int s = bitmap.firstFit(1);
        bitmap.allocate(s, 1);

        for (int i = 0; i < numBlocks - 1; i++) {
            Block block {buffer, i * NUM_BYTES_PER_BLOCK, std::min(bytes, NUM_BYTES_PER_BLOCK)};

            // Get next available block
            int l = bitmap.firstFit(1);
            blockIndexes.push_back(l);

            // Allocate block on disk
            disk.assignBlock(block, l);

            // Allocate block on bitmap
            bitmap.allocate(l, 1);

            bytes -= NUM_BYTES_PER_BLOCK;
        }
        
        // Entry block
        Block block;
        for (int i = 0; i < blockIndexes.size(); i++) {
            block.setByte(blockIndexes[i], i);
        }

        // Save entry block to disk
        disk.assignBlock(block, s);

        // Save entry to file allocation table
        fat.add(name, s, numBlocks, buffer.size());
    }

    std::vector<char> getFileData(const std::string& name) override {
        // Buffer for holding file data
        std::vector<char> buffer;

        // Get block start and length
        std::tuple<int, int, int> t = fat.get(name);
        
        int blockIndex = std::get<0>(t);
        int numBlocks = std::get<1>(t);
        int numBytes = std::get<2>(t);

        // Entry block
        Block entryBlock = disk.getBlock(blockIndex);
        
        // Transfer blocks of data to buffer
        for (int i = 0; i < numBlocks - 1; i++) {
            blockIndex = entryBlock.getByte(i);
            Block block = disk.getBlock(blockIndex);

            block.copy(buffer, std::min(numBytes, NUM_BYTES_PER_BLOCK));
            numBytes -= NUM_BYTES_PER_BLOCK;
        }

        return buffer;
    }

};


#endif


