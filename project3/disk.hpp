#ifndef DISK_HPP
#define DISK_HPP

#include "types.hpp"
#include <vector>

class Block {
public:
    Block() {
        for (int i = 0; i < NUM_BYTES_PER_BLOCK; i++) {
            bytes[i] = 0;
        }
    }

    Block(const std::vector<char>& buffer, int numBytes) {
        for (int i = 0; i < numBytes; i++) {
            this->bytes[i] = buffer[i];
        }
        for (int i = numBytes; i < NUM_BYTES_PER_BLOCK; i++) {
            this->bytes[i] = 0;
        }
    }

    Block(const std::vector<char>& buffer, int l, int numBytes) {
        for (int i = 0; i < numBytes; i++) {
            this->bytes[i] = buffer[l + i];
        }
        for (int i = numBytes; i < NUM_BYTES_PER_BLOCK; i++) {
            this->bytes[i] = 0;
        }
    }

    Block(const Block& rhs) {
        for (int i = 0; i < NUM_BYTES_PER_BLOCK; i++) {
            bytes[i] = rhs.bytes[i];
        }
    }

    void copy(std::vector<char>& buffer, int numBytes) {
        for(int i = 0; i < numBytes; i++) {
            buffer.push_back(bytes[i]);
        }
    }

    void setByte(byte byte, int pos) {
        bytes[pos] = byte;
    }

    byte getByte(int pos) {
        return bytes[pos];
    }

private:
    byte bytes[NUM_BYTES_PER_BLOCK];
};

class Disk {
public:
    void assignBlock(Block block, int blockNumber) {
        blocks[blockNumber] = block;
    }

    Block getBlock(int blockNumber) {
        return blocks[blockNumber];
    }

private:
    Block blocks[NUM_BLOCKS];

};


#endif


