
#ifndef BITMAP_HPP

#define BITMAP_HPP

#include "types.hpp"

class Bitmap {
public:
    Bitmap() {
        for (int i = 0; i < NUM_BLOCKS; i++) {
            bm[i] = false;
        }
    }
    
    void display();

    // FIXME: add updates to disk block when this gets updated

    void allocate(int l, int n) {
        // Mark blocks from i to i + n - 1 as allocated
        for (int i = l; i < l + n; i++){
            if (bm[i]) {
                // FIXME: error
            }
            bm[i] = true;
        }
    }

    void deallocate(int l, int n) {
        for (int i = l; i < l + n; i++) {
            if (!bm[i]) {
                // FIXME: error
            }
        }
        bm[i] = false;
    }

    int firstFit(int n) {
        /*
            Finds the first contiguous set of n blocks that are not allocated

            Returns index of first block in contiguous set (0 - 255)
            Returns -1 if contiguous set doesn't exist
        */

        int l = 0;

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

private:
    bool bm[NUM_BLOCKS];

};


#endif
