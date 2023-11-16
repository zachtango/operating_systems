
#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include "disk.hpp"
#include "bitmap.hpp"
#include "fileTable.hpp"
#include "types.hpp"
#include <string>

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
        getFileData(name);

        // write file to real computer

    }

    void copyFromComputer(const std::string& src, const std::string& name) {
        // get file contents from computer
        byte* fileData;

        // write file to disk
        allocateFile(name, 0, fileData);

    }
    
    virtual void deleteFile(const std::string& name) = 0;

protected:
    virtual void allocateFile(const std::string& name, int bytes, byte* byteArray) = 0;
    virtual void getFileData(const std::string& name) = 0;


    Disk disk;
    Bitmap bitmap;
    FileTable fat;

};

class ContiguousFileSystem : FileSystem {

    void allocateFile(const std::string& name, int bytes, byte* byteArray) override {
        int numBlocks = bytes / NUM_BYTES_PER_BLOCK;

        int l = bitmap.firstFit(numBlocks);

        if (l == -1) {
            // FIXME: not enuf space err
            return;
        }

        bitmap.allocate(l, numBlocks);
        fat.add(name, l, numBlocks);

        // transfer data to disk
    }

    void deleteFile(const std::string& name) override {
        
        // get index and length from fat
        // delete in disk
        int l, numBlocks;

        bitmap.deallocate(l, numBlocks);
        
        fat.del(name);
    }

    void compact() {
        
    }

};

class ChainedFileSystem : FileSystem {

};

class IndexedFileSystem : FileSystem {

};


#endif


