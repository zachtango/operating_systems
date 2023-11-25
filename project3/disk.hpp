#ifndef DISK_HPP
#define DISK_HPP

#include "types.hpp"
#include <vector>

class Block {
public:
    Block() {
        // Initialize blocks to 0
        for (int i = 0; i < NUM_BYTES_PER_BLOCK; i++) {
            bytes[i] = 0;
        }
    }

    Block(const std::vector<char>& buffer, int numBytes) {
        // Put buffer in block
        for (int i = 0; i < numBytes; i++) {
            this->bytes[i] = buffer[i];
        }
        // Fill rest of block with 0s
        for (int i = numBytes; i < NUM_BYTES_PER_BLOCK; i++) {
            this->bytes[i] = 0;
        }
    }

    Block(const std::vector<char>& buffer, int l, int numBytes) {
        // Put buffer in block
        for (int i = 0; i < numBytes; i++) {
            this->bytes[i] = buffer[l + i];
        }
        // Fill rest of block with 0s
        for (int i = numBytes; i < NUM_BYTES_PER_BLOCK; i++) {
            this->bytes[i] = 0;
        }
    }

    Block(const Block& rhs) {
        // Copy constructor
        for (int i = 0; i < NUM_BYTES_PER_BLOCK; i++) {
            bytes[i] = rhs.bytes[i];
        }
    }

    void copy(std::vector<char>& buffer, int numBytes) {
        // Copy block to buffer
        for(int i = 0; i < numBytes; i++) {
            buffer.push_back(bytes[i]);
        }
    }

    void setByte(byte byte, int pos) {
        // Set a specific byte in the block
        bytes[pos] = byte;
    }

    byte getByte(int pos) {
        // Get a specific byte in the block
        return bytes[pos];
    }

private:
    byte bytes[NUM_BYTES_PER_BLOCK];
};

class Disk {
public:
    void assignBlock(Block block, int blockNumber) {
        // Assign a certain block in the disk
        blocks[blockNumber] = block;
    }

    Block getBlock(int blockNumber) {
        // Get a certain block in the disk
        return blocks[blockNumber];
    }

private:
    Block blocks[NUM_BLOCKS];

};


#endif


