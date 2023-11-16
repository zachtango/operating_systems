#ifndef DISK_HPP
#define DISK_HPP

#include "types.hpp"

class Block {
public:
    


private:
    byte bytes[NUM_BYTES_PER_BLOCK];

};

class Disk {
public:

private:
    Block blocks[NUM_BLOCKS];
    

    
};


#endif


