
#ifndef BITMAP_HPP

#define BITMAP_HPP

#include "types.hpp"
#include <iostream>
#include <exception>

class Bitmap {
public:
    Bitmap() {
        // Initialize empty bitmap
        for (int i = 0; i < NUM_BLOCKS; i++) {
            bm[i] = false;
        }

        // Reserve space for 
        bm[0] = true;
        bm[1] = true;
    }
    
    void display() {
        // Title of bitmap
        std::cout << ' ';
        for (int i = 0; i < 5; i++) {
            std::cout << "-";
        }
        std::cout << "BITMAP";
        for(int i = 0; i < 5; i++) {
            std::cout << "-";
        }
        std::cout << '\n';

        // Body of bitmap
        for (int i = 0; i < 16; i++) {
            std::cout << '|';
            for (int j = 0; j < 16; j++) {
                std::cout << (bm[i * 16 + j] ? "x" : "o");
            }
            std::cout << "|\n";
        }

        std::cout << ' ';
        for (int i = 0; i < 16; i++) {
            std::cout << "-";
        }
        std::cout << "\n\n";
    }

    void allocate(int l, int n) {
        // Allocate blocks from i to i + n - 1
        for (int i = l; i < l + n; i++){
            if (bm[i]) {
                throw std::runtime_error("Allocating reserved block\n");
            }
            bm[i] = true;
        }
    }

    void deallocate(int l, int n) {
        // Deallocate blocks from i to i + n - 1
        for (int i = l; i < l + n; i++) {
            if (!bm[i]) {
                throw std::runtime_error("Deallocating unallocated block\n");
            }
            bm[i] = false;
        }
    }

    int firstFit(int n) {
        /*
            Finds the first contiguous set of n blocks that are not allocated

            Returns index of first block in contiguous set (0 - 255)
            Returns -1 if contiguous set doesn't exist
        */

        int l = 2;

        while (true) {
            while (l < NUM_BLOCKS && bm[l]) {
                l += 1;
            }

            if (l == NUM_BLOCKS) {
                break;
            }

            // l is at an open block
            int r = l;

            while (r < NUM_BLOCKS && (r - l + 1) < n && !bm[r]) {
                r += 1;
            }
            
            if ((r - l + 1) == n) {
                return l;
            }

            l = r;
        }

        return -1;
    }

    int numOpenBlocks() {
        // Count the number of open blocks that can be allocated

        int counter = 0;
        for (int i = 2; i < NUM_BLOCKS; i++) {
            if (!bm[i]) {
                counter += 1;
            }
        }
        return counter;
    }

private:
    bool bm[NUM_BLOCKS];
};


#endif
