#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "copyright.h"
#include "bitmap.h"
//#include "addrspace.h"


class MemoryManager {
 public:
  MemoryManager();
  ~MemoryManager();
  int getPage(int vPgNum);
  void clearPage(int i);
  int availablePages();
  int findOldestEntry();
  void writeToSwap(int vPageNum, void * ad);
  void sendToSwap(int vPageNum, void * a);
  
 private:
  BitMap * pageMap;
  struct CoreMapEntry {
    bool allocated;
    unsigned int vPageNum;
    void * a; 
    unsigned int fifoNum;
    bool lockedInIO;     // Come back to this later
  };
  CoreMapEntry ** coreRecord;
  unsigned int fifoTracker;
  
};
#endif
