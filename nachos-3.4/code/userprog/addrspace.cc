// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#ifdef HOST_SPARC
#include <strings.h>
#endif

#define INVALID_ALLOCATION 1
#define NO_PARENT -1
#define NOT_IN_MEM -2

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(PCB * p) {
  numPages = -1;
  pcb = p;
  pageTable = NULL;
}

AddrSpace::AddrSpace(OpenFile *executable, bool createPCB, char * fileName, PCB * p)
{
  unsigned int i, size;
  
  executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
  if ((noffH.noffMagic != NOFFMAGIC) && 
      (WordToHost(noffH.noffMagic) == NOFFMAGIC))
    SwapHeader(&noffH);
  ASSERT(noffH.noffMagic == NOFFMAGIC);
  

  size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
    + UserStackSize;	// we need to increase the size
  
  numPages  = divRoundUp(size, PageSize);  
  size = numPages * PageSize;

  printf("The size is [%d]\n", size);
  printf("Pages needed [%d]\n", numPages);


  if (createPCB)
    pcb = processManager->createPCB(NO_PARENT,currentThread, false);
  else
    pcb = currentThread->space->pcb;
  
  DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
	numPages, size);

  // if(memoryManager->availablePages() < numPages) {
  //   printf("Not Enough Memory\n");
  //   DEBUG('a',"Over allocation has occured. Cleaning up!\n");
  //   printf("Lets try swapping!\n");
  // }
  
  pageTable = new TranslationEntry[numPages];
  for (i = 0; i < numPages; i++) {
    pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
    pageTable[i].physicalPage = NOT_IN_MEM;
    
    /* With valid set to false we can now create a page fault */
    pageTable[i].valid = FALSE;
    pageTable[i].use = FALSE;
    pageTable[i].dirty = FALSE;
    pageTable[i].readOnly = FALSE; 
    pageTable[i].inMem = FALSE;
    pageTable[i].sharedEntry = NULL;
  }

  // if (createPCB) 
  //   bzero(machine->mainMemory, NumPhysPages * PageSize);

  if (p != NULL)
    pcb = p;

  if (fileName != NULL) {
    // printf ("File is %s\n", fileName);
    exec = fileSystem->Open(fileName);
    initSwap(exec, &swapFile, true, noffH.code.inFileAddr);
  } else {
    // printf ("File is %s\n", fileName);
    initSwap(executable, &swapFile, true, noffH.code.inFileAddr);
  }
  printf("Loaded Program: [%d] code | [%d] data | [%d] bss\n",
	   noffH.code.size, noffH.initData.size, noffH.uninitData.size);	
}


//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{

  unsigned int i;

  // printf("pageTable %d\n", pageTable);
  // printf(">>>>>>>>>>>>>>>>>>>>>> Space being deallocated belongs to %d\n",
  // 	 currentThread->space->pcb->getMyPid(), i);

  if (pageTable != NULL) {
    for (i = 0; i < numPages; i++) {
      //      pageTable[i].valid = FALSE;
      if(pageTable[i].sharedEntry != NULL) {
         RemoveFromSharedList(i);
      } else if (pageTable[i].inMem == true) {
	       memoryManager->clearPage(pageTable[i].physicalPage);
      }
    }

    RestoreState();
    delete pageTable;
    pageTable = NULL;
  }
  
  if (pcb != NULL)
    processManager->clearPid(pcb->getMyPid());  
  
  //delete exec;
  //  fileSystem->Remove(name);
  delete swapFile;

}

void
AddrSpace::evictPages() {

  unsigned int k;
  for (k = 0; k < numPages; k++) 
    if(pageTable[i].sharedEntry != NULL) {
         RemoveFromSharedList(i);
    } else if (pageTable[k].inMem == true)  {
      memoryManager->writeToSwap(pageTable[k].virtualPage, this);
      memoryManager->clearPage(pageTable[k].physicalPage);
    }
	
  delete pageTable;
  pageTable = NULL;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() {}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

void
AddrSpace::sendToMem(int virtAddr)
{
  unsigned int ptIndex;//, pOffset;
  ptIndex = (unsigned) virtAddr / PageSize; 

  int inSwapFileAddr = ptIndex * PageSize;

  int totalSize = noffH.code.size + noffH.initData.size
    + noffH.uninitData.size + UserStackSize;

  printf ("^^ We are putting data in %d ^^ \n", ptIndex);
  pageTable[ptIndex].physicalPage
    =  memoryManager->getPage(pageTable[ptIndex].virtualPage);
  pageTable[ptIndex].valid = TRUE;
  pageTable[ptIndex].inMem = TRUE;

  // int readSize = min (totalSize - inSwapFileAddr, PageSize);
  ReadFile(inSwapFileAddr, swapFile, PageSize, inSwapFileAddr);
  
  printf("L [%d]: [%d] -> [%d]\n", pcb->getMyPid(),
	 pageTable[ptIndex].virtualPage, pageTable[ptIndex].physicalPage); 


  if(ptIndex == 11) {
    printf("sendToMem.  read to page 11. OpenFile Pointer %d:\n", (int)swapFile);
    for(int i = 0; i < 128; i += 4) {
      printf("%d ", machine->mainMemory[pageTable[ptIndex].physicalPage*PageSize+i]);
    }
    printf("\n");
  }

  if(pageTable[ptIndex].sharedEntry != NULL) {
    SendSharedToMem(ptIndex);
  }

  RestoreState();
}

void
AddrSpace::invalidateByVPage(int i)
{
  RemoveFromSharedList(i);
  pageTable[i].physicalPage = NOT_IN_MEM;
      
  /* With valid set to false we can now create a page fault */
  pageTable[i].valid = FALSE;
  pageTable[i].readOnly = FALSE; 
  pageTable[i].inMem = FALSE; 
}

void
AddrSpace::invalidateByPhysPage(int i)
{
  unsigned int k ;
  for (k = 0; k < numPages; k++) {
    if (pageTable[k].physicalPage == i) {
      RemoveFromSharedList(k);
      pageTable[k].physicalPage = NOT_IN_MEM;   
      /* With valid set to false we can now create a page fault */
      pageTable[k].valid = FALSE;
      pageTable[k].readOnly = FALSE; 
      pageTable[k].inMem = FALSE;      
    }
  }
}

bool
AddrSpace::Translate(int virtAddr, int * physAddr)
{

  // printf("\n\n#### PID: [%d] invoked %s\n\n\n\n",
  //  currentThread->space->pcb->getMyPid()
  // 	 , "TRANSLATING");
  // printf("The virtAddr is %d", virtAddr);
  unsigned int ptIndex, pOffset;
  ptIndex = (unsigned) virtAddr / PageSize;
  pOffset = (unsigned) virtAddr % PageSize;

  DEBUG('a', "ptIndex is %d, pOffset is %d!\n",
	ptIndex, pOffset);

  if (ptIndex >= numPages) {
    DEBUG('a', "Virtual Page # %d too large for page table size %d!\n",
	  virtAddr, numPages);
    printf( "Virtual Page # %d too large for page table size %d!\n",
	  virtAddr, numPages);
    return false;
  } else if (pageTable[ptIndex].valid == false &&  pageTable[ptIndex].inMem == true) {
    DEBUG('a', " Attempting to translate invalid page!\n");
    printf( " Attempting to translate invalid page!\n");
    return false;
  } else if (pageTable[ptIndex].physicalPage == -1) {
    DEBUG('a', "Attempting to translate when there is no frame!\n");
    printf ("Attempting to translate when there is no frame!\n");
    return false;
  }

  // printf("Value of physical page is %d\n", pageTable[ptIndex].physicalPage);
  // printf("Value of physical page offset is %d\n", pOffset);
  //printf("phys addr = %d\n", (int) physAddr);

  
  
  * physAddr = (pageTable[ptIndex].physicalPage * PageSize) + pOffset;

  printf("Value of physical page is %d\n", pageTable[ptIndex].physicalPage);
  printf("Value of physical page offset is %d\n", pOffset);
  printf("phys addr value = %d\n", (pageTable[ptIndex].physicalPage * PageSize) + pOffset);
  DEBUG('a', "phys addr = 0x%x\n",     physAddr);
  DEBUG('a', "phys addr = %d\n", (int) physAddr);
  // printf("phys frame = %d\n", ((int) physAddr ) / PageSize);
  
  if( ((int)*physAddr < 0) || (int)*physAddr > MemorySize ) {
    DEBUG('a', "Translation results in a location outside of memory!\n");
    printf ("Translation results in a location outside of memory!\n");
    return false;
  }

  DEBUG('a', "Translation Succeded!\n");
  return true;
}

void
AddrSpace::initSwap(OpenFile * file, OpenFile ** swap, bool empty, int offset)
{
  char swapName[255];
  int r = Random();
  int writeAddr = 0;
  char * emptyStack = new char[UserStackSize*2];
  
  if (empty)
    bzero (emptyStack, UserStackSize*2);

  //  if (empty)
  sprintf(swapName, "swap_%d_%d", pcb->getMyPid(), r);

  int size = noffH.code.size + noffH.initData.size + noffH.uninitData.size;	// we need to increase the size
  
  // numPages  = divRoundUp(size, PageSize);  
  // size = numPages * PageSize;
  
  // fileSystem->Remove(swapName);

  if (fileSystem->Create(swapName, 0))
    *swap = fileSystem->Open(swapName);
  
  if (swap != NULL) {
    // printf("noffH.code.size is %d\n",  noffH.code.size);
    writeAddr = mkSwap(file, *swap,
		       size, offset,
		       writeAddr);
    offset += size;
    // printf("noffH.initData.size is %d\n",  noffH.initData.size);
    
    // writeAddr = mkSwap(file, *swap,
		  //      noffH.initData.size, offset,
		  //      writeAddr);
    // offset += noffH.initData.size;
    // // printf("noffH.uninitData.size is %d\n",  noffH.uninitData.size);
    
    // writeAddr = mkSwap(file, *swap,
		  //      noffH.uninitData.size, offset,
		  //      writeAddr);
    // offset += noffH.uninitData.size;

    if(empty) {
      int stacksize = size + UserStackSize;
      numPages  = divRoundUp(stacksize, PageSize);  
      size = numPages * PageSize - size;

      (*swap)->WriteAt(emptyStack, size, writeAddr);
      writeAddr += size;

    } else {
       writeAddr = mkSwap(file, *swap, UserStackSize,
			  offset, writeAddr);


      int stacksize = size + UserStackSize;
      numPages  = divRoundUp(stacksize, PageSize);  
      size = numPages * PageSize - size;
      (*swap)->WriteAt(emptyStack, size - UserStackSize, writeAddr);
      writeAddr += size;

    }
  }

  bcopy(swapName, name, strlen(swapName));
}

int
AddrSpace::mkSwap(OpenFile * file, OpenFile * swap, int size, int fileAddr, int writeAddr)
{
  if(size <= 0) {
    return writeAddr;
  }
  int bytesToCopy;
  char * buf = new char[size];
  file->ReadAt(buf, size, fileAddr);
  swap->WriteAt(buf, size, writeAddr);
  writeAddr += size;
  // char * buf = new char[PageSize];

  
  // while (bytesRemaining) {
  //   bytesToCopy = (size < PageSize) ? size : PageSize;
  //   file->ReadAt(buf, bytesToCopy, fileAddr);
  //   swap->WriteAt(buf, bytesToCopy, writeAddr);
  //   DEBUG('a', "Bytes that will be read %d\n", bytesToCopy);
        
  //   size      -= bytesToCopy;
  //   fileAddr  += bytesToCopy;
  //   writeAddr += bytesToCopy;
    
  //   bytesRemaining = (size > 0) ? true : false; 
  //   DEBUG('a', "size %d, fileAddr %d bytesRemaining %s\n"
	 //  ,size, fileAddr, bytesRemaining ? "true" : "false");
  // }

  delete [] buf;
  return writeAddr;
}

int
AddrSpace::ReadFile(int virtAddr, OpenFile * file, int size, int fileAddr)
{
  bool bytesRemaining = (size > 0) ? true : false;
  int physAddr, bytesToCopy;
  char * buf = new char[PageSize];

  while (bytesRemaining) {
    physAddr = -1;
    bytesToCopy = (size < PageSize) ? size : PageSize;
    file->ReadAt(buf, bytesToCopy, fileAddr);
    // Read 1 pg at a time or the final offset

    DEBUG('a', "Bytes that will be read %d\n", bytesToCopy);
    if (!Translate(virtAddr, &physAddr)) {
      DEBUG('a', "ABNORMAL Translation result: VirtAddr %d, PhysAddr %d\n"
	    , virtAddr, physAddr);
      printf("ABNORMAL Translation result: VirtAddr %d, PhysAddr %d\n"
	    , virtAddr, physAddr);
      return -1;
    }    
    bcopy(buf, machine->mainMemory + physAddr, bytesToCopy);
    
    size     -= bytesToCopy;
    virtAddr += bytesToCopy;
    fileAddr += bytesToCopy;
    
    bytesRemaining = (size > 0) ? true : false; // If we've exhausted the size finish
    DEBUG('a', "size %d, virtAddr %d, fileAddr %d bytesRemaining %s\n"
	  ,size, virtAddr, fileAddr,  bytesRemaining ? "true" : "false");
  }
  
  delete [] buf;
  return bytesRemaining;
}

void
AddrSpace::Fork(AddrSpace * a)
{
  if(memoryManager->availablePages() < numPages) {
    printf("Not Enough Memory. Sending to swap\n");
    DEBUG('a',"We need to page fault\n");
  }
  
  
  TranslationEntry * fpageTable
    = new TranslationEntry[numPages];

  /* Implement the functionality here from the default constructpr */
  unsigned int i;
  for (i = 0; i < numPages; i++) {
    if (pageTable[i].inMem == TRUE) {
      memoryManager->sendToSwap(pageTable[i].virtualPage, (void *) this);
    }
  }
  
  for (i = 0; i < numPages; i++) {
    fpageTable[i].virtualPage  = i;	
    fpageTable[i].physicalPage = NOT_IN_MEM;
    fpageTable[i].valid    = FALSE;
    fpageTable[i].use      = pageTable[i].use; //FALSE;
    fpageTable[i].dirty    = pageTable[i].dirty; //FALSE;
    fpageTable[i].readOnly = TRUE;
    fpageTable[i].inMem    = FALSE;
    ShareVPage(a, fpageTable[i], this, pageTable[i]);

    DEBUG('a', "Page table attributes:\n");
    DEBUG('a', "POPULATE ATTRIBUTES HERE\n");
  }

  // int totalSize = noffH.code.size + noffH.initData.size
  //   + noffH.uninitData.size + UserStackSize;
  
  
  a->pageTable = fpageTable;
  a->numPages = numPages;
  //  a->swapFile = swapFile;
  a->noffH = noffH;
  a->initSwap(swapFile, &a->swapFile, false, 0);
}

int
AddrSpace::userReadWrite(char * buf, int virtAddr, int size, bool reading)
{
  bool bytesRemaining = (size > 0) ? true : false;
  int physAddr;
  int bytesToCopy;
  int bytesRead = 0;
  int k = 0;
  bool quickExit = false;
  
  while (bytesRemaining && !quickExit) {
    physAddr = -1;
    bytesToCopy = (size < PageSize) ? size : PageSize;
    if (!Translate(virtAddr, &physAddr)) 
      return -1;

    if(reading) {
      for (k = bytesRead ; k < bytesRead + bytesToCopy ; k++) {
	if (buf[k] == '\0')
	  {
	    bytesToCopy = k - bytesRead;
	    quickExit = true;
	    break;  
	  }
      }
      bcopy(buf, machine->mainMemory + physAddr, bytesToCopy);
    } else {
      for (k = physAddr ; k < physAddr + bytesToCopy ; k++) {
	if (*(machine->mainMemory + k) == '\0')
	  {
	    bytesToCopy = k - physAddr;
	    quickExit = true;
	    break;  
	  }
      }
      bcopy(machine->mainMemory + physAddr, buf, bytesToCopy);
    }

    bytesRead += bytesToCopy;
    size      -= bytesToCopy;
    virtAddr  += bytesToCopy;
    bytesRemaining = (size > 0) ? true : false;   
  }

  return bytesRead;
}

int AddrSpace::GetNumPages() {
  return numPages;
}

void AddrSpace::ShareVPage(Addrspace * currentSpace, TranslationEntry& current, Addrspace * otherSpace, TranslationEntry& other) {
  if(other.sharedEntry == NULL) {
    other.sharedEntry = CreateShareEntry();
    other.sharedEntry->sharedList->Append((void *) &other);
    other.readOnly = TRUE; 
  }

  current.sharedEntry = other.sharedEntry;
  current.sharedEntry->sharedList->Append((void *) &current);
  current.readOnly = TRUE;

}

SharedTranslationEntry* AddrSpace::CreateShareEntry() {
    SharedTranslationEntry* entry = new SharedTranslationEntry*();
    entry->sharedEntry->sharedList = new List;
    return entry;
}

static void
RemoveFromShared(int entryPointer)
{
  TranslationEntry* entry = (TranslationEntry*)entryPointer;
  RemoveFromSharedList(entry);
}

void AddrSpace::RemoveFromSharedList(TranslationEntry* entry) {
  TranslationEntry* deleted = (TranslationEntry*) entry->sharedEntry->sharedList->RemoveByKey((int)entry);
  if(deleted != entry) {
    printf("!!error: RemoveFromSharedList  removed wrong space");
    return;
  }
  if(entry->IsEmpty()) {
    delete entry->sharedEntry;
  } else if(entry->sharedEntry->isSingleton()) {
    //delete shared entry
    entry->sharedEntry->Mapcar(RemoveFromShared);
  }
  entry->sharedEntry = NULL;
  entry->readOnly = FALSE;
}


static void
SharePhysPage(int entryPointer, int physicalPage)
{
  TranslationEntry* entry = (TranslationEntry*)entryPointer;
  entry->physicalPage = physicalPage;
  entry->valid = TRUE;
  entry->inMem = TRUE;
}

void AddrSpace::SendSharedToMem(int vAddr) {
  pageTable[i].sharedEntry->sharedList->mapcarInt(SharePhysPage, pageTable[i].physicalPage);
}
