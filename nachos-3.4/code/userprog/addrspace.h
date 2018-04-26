// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "noff.h"
#include "copyright.h"
#include "filesys.h"
#include "pcb.h"

#define UserStackSize		1024 	// increase this as necessary!

class AddrSpace {
  public:
  AddrSpace(OpenFile *executable,
	    bool createPCB = true,
	    char * fileName = NULL,
	    PCB * p = NULL);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch 
    bool Translate(int virtAddr, int * phyAddr);
    void initSwap(OpenFile * file, OpenFile ** swap, bool empty, int offset);
    int mkSwap(OpenFile * file, OpenFile * swap, int size,
	       int fileAddr, int writeAddr);
    int ReadFile(int virtAddr, OpenFile * file,
		 int size, int fileAddr);
    AddrSpace::AddrSpace(PCB * p);
    void Fork(AddrSpace * a);
    int GetNumPages();
    PCB* pcb;
    char name[255];
    OpenFile * swapFile;
    bool child;
    int userReadWrite(char * buf, int virtAddr, int size, bool reading);
    void sendToMem(int vAddr);
    void cpySwap(OpenFile * file, OpenFile * swap, int size, int fileAddr);
    void invalidateByVPage(int i);
    void invalidateByPhysPage(int i);
    void evictPages();
    void appendSwap(int virtAddr);
    NoffHeader noffH;
    OpenFile * exec;
    
    void ShareVPage(AddrSpace * currentSpace, TranslationEntry& current, AddrSpace * otherSpace, TranslationEntry& other);
    SharedTranslationEntry* CreateShareEntry();
    static void RemoveFromSharedList(TranslationEntry* entry) ;
    void SendSharedToMem(int vAddr);
    void SeperateFromShared(int vAddr);
  private:
    TranslationEntry *pageTable;	// Assume linear page table translation
					// for now!
    unsigned int numPages;		// Number of pages in the virtual 
					// address space
    bool initial;
    int inSwapFileAddr;
    int inSwapStackAddr;
    int pageCount;
};
#endif // ADDRSPACE_H
