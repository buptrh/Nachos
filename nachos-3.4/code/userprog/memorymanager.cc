#include "memorymanager.h"
#include "system.h"
#define NO_FREE_PAGE -1

MemoryManager::MemoryManager()
{
  pageMap = new BitMap (NumPhysPages);
  coreRecord = new CoreMapEntry*[NumPhysPages];
  fifoTracker = 0;
}

MemoryManager::~MemoryManager()
{
  delete pageMap;
  delete [] coreRecord;
}

int
MemoryManager::getPage(int vPgNum, int offset)
{
  int candidatePage = -1;

  // Ensure Atomic Operations on memory manager
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
    
  candidatePage = pageMap->Find();

  /* Check if candidatePage is invalid and if not alloc and record */
  /* Otherwise select a victim ! */

  /* Construct our new frame */
  CoreMapEntry * entry = new CoreMapEntry;

  entry->allocated = true;
  entry->vPageNum = vPgNum; // Need to get virt page num here
  entry->a = (void *) currentThread->space;
  entry->fifoNum = fifoTracker;
  entry->stackOffset = offset;

  /* Increment the tracker for the next page */
  if (candidatePage != NO_FREE_PAGE) {
    
    if (coreRecord[candidatePage] == NULL) 
      coreRecord[candidatePage] = entry;
    else 
      DEBUG('a', "Our map is incorrect execution should never flow here");

    bzero(machine->mainMemory + (candidatePage * PageSize), PageSize);   
  } else {
    if (fifoTracker == NumPhysPages) 
           fifoTracker = 0;
          
    int victim = fifoTracker; 
      
    /* Move the victim to the backing store */
    printf("S [%d]: [%d]\n",
	   currentThread->space->pcb->getMyPid(),
	   victim);
    
    sendToSwap(coreRecord[victim]->vPageNum, coreRecord[victim]->a);

    
    /* Clear entry */
    /* Load entry */
    pageMap->Mark(victim);
    coreRecord[victim] = entry;
    
    bzero(machine->mainMemory + (victim * PageSize), PageSize);
    candidatePage = victim; // Clean this up later
    // int va = vPageNum * PageSize;
        
    fifoTracker++;
    //printf("candidatePage is %d\n+++++WE need to sendToMem do we?", candidatePage);
  }
  // 
  (void) interrupt->SetLevel(oldLevel);
  return candidatePage;
}

// Only call when interrupts are off
int
MemoryManager::findOldestEntry()
{
  unsigned int min = 4294967295; // Make this UINT_MAX 
  int k = -1;
  int i;
  
  for (i = 0 ; i < NumPhysPages ; i++) {
    if (coreRecord[i] != NULL) {

      //      printf("fifoNum is [%d]\n", coreRecord[i]->fifoNum);
      
      if (min > coreRecord[i]->fifoNum && !coreRecord[i]->lockedInIO) {
	min = coreRecord[i]->fifoNum;
	k = i;
      }
    }
  }

  return k;
}

void
MemoryManager::clearPage(int i)
{
  // Ensure Atomic Operations on memory manager
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  if(pageMap->Test(i) && coreRecord[i] != NULL) {
    printf("E [%d]: [%d]\n",
    	   currentThread->space->pcb->getMyPid(), i);
    
    pageMap->Clear(i);
    ((AddrSpace *) coreRecord[i]->a)->invalidateByPhysPage(i);    
    coreRecord[i] = NULL;
    bzero(machine->mainMemory + (i * PageSize), PageSize);
    
  } else {
    DEBUG('p', "clearPage failed page %d\n", i );
  }
  (void) interrupt->SetLevel(oldLevel);
}

int
MemoryManager::availablePages()
{
  return pageMap->NumClear();
}

void
MemoryManager::writeToSwap(int vPageNum, void * ad)
{
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  
  int offset = vPageNum * PageSize;
  char * buf = new char[PageSize];
  int physAddr = -1;
  AddrSpace * a = (AddrSpace *) ad;

  if (a->Translate(offset, &physAddr)) {
    int i = 0;
    for ( ; i < NumPhysPages ; i++) {
      if (coreRecord[i] != NULL)
	if (coreRecord[i]->vPageNum == vPageNum
	    && coreRecord[i]->stackOffset != -1) 
	  offset = coreRecord[i]->stackOffset;
    }
    
    a->swapFile->WriteAt(buf, PageSize, offset);
  }
  (void) interrupt->SetLevel(oldLevel);
}

void
MemoryManager::sendToSwap(int vPageNum, void * ad)
{
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  
  int offset = vPageNum * PageSize;
  char * buf = new char[PageSize];
  int physAddr = -1;
  AddrSpace * a = (AddrSpace *) ad;

  printf("!@#@#@%Before if we are trying to translate vpn %d\n", vPageNum);
  if (a->Translate(offset, &physAddr)) {
    int i = 0;
    for ( ; i < NumPhysPages ; i++) {
      if (coreRecord[i] != NULL)
	if (coreRecord[i]->vPageNum == vPageNum
	    && coreRecord[i]->stackOffset != -1) 
	  offset = coreRecord[i]->stackOffset;
    }
    

    bcopy (machine->mainMemory + physAddr, buf, PageSize);
    a->invalidateByVPage(vPageNum);    
    a->swapFile->WriteAt(buf, PageSize, offset);
    a->swapFile->ReadAt(buf, PageSize, offset);
  }
    pageMap->Clear(physAddr / PageSize);
    delete coreRecord[physAddr / PageSize];
    coreRecord[physAddr / PageSize] = NULL;
  (void) interrupt->SetLevel(oldLevel);
}
