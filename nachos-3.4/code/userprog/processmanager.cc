#include <processmanager.h>
#include <system.h>


ProcessManager::ProcessManager()
{
  pcbRecord = new PCB*[PCB_ENTRIES];
  master = new BitMap(PCB_ENTRIES);
}

ProcessManager::~ProcessManager()
{
  delete [] pcbRecord;
  delete master;
}

PCB *
ProcessManager::createPCB(int parentPid, Thread * t, bool forked) { 

  // Ensure Atomic Operations on memory manager
  IntStatus oldLevel = interrupt->SetLevel(IntOff);

  int candidatePid = getPid(parentPid);
  if(candidatePid == -1) {
    return NULL;
  }

  // This is the child process
  PCB * pcb = new PCB(candidatePid, parentPid, (void *)t); 
  pcbRecord[candidatePid] = pcb;
  if (forked)
    currentThread->space->pcb->registerChild(candidatePid);
  
  (void) interrupt->SetLevel(oldLevel);

  return pcb;
}

int
ProcessManager::getPid(int parentPid)
{
  int candidatePid = -1;
  candidatePid = master->Find(); 
  return candidatePid;
}

bool
ProcessManager::hasMultiThreads(int i)
{
  if (master->Test(i)) 
    if (pcbRecord[i] != NULL) 
      return pcbRecord[i]->isMultiThreaded();

  return false;
}


void
ProcessManager::clearPid(int i)
{
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  if (master->Test(i)) {
    if (hasMultiThreads(i)) {

      pcbRecord[i]->removeThread(currentThread);
      return;
    }
    if (pcbRecord[i]->hasChildren()) {
      pcbRecord[i]->removeParent();
    }
    if(pcbRecord[i]->getParent() != -1) {
      pcbRecord[pcbRecord[i]->getParent()]->removeChild(i);
    }

    master->Clear(i);
    delete pcbRecord[i];
    pcbRecord[i] = NULL;
  }
  (void) interrupt->SetLevel(oldLevel);
}

PCB *
ProcessManager::getPCB(int i)
{
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  if (master->Test(i) && pcbRecord[i] != NULL) {
    return pcbRecord[i];
  }
  return NULL;
  (void) interrupt->SetLevel(oldLevel);
}

void
ProcessManager::addThread(int pid, Thread * t)
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    int i = pid;
    
    if (master->Test(i)) 
      if (pcbRecord[i] != NULL) 
	pcbRecord[i]->registerThread((void *) t);
    
    (void) interrupt->SetLevel(oldLevel);
}

void
ProcessManager::endThread(int pid, Thread * t)
{
  int i = pid;
    
  if (master->Test(i) && (pcbRecord[i] != NULL)) 
      pcbRecord[i]->removeThread((void *) t);
    
}

void
ProcessManager::endProcess(int i)
{
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  if (master->Test(i) && pcbRecord[i] != NULL) {
      pcbRecord[i]->suicide();
  }
  (void) interrupt->SetLevel(oldLevel);
}

void
ProcessManager::endOtherThreads(Thread * t)
{
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  int i =  currentThread->space->pcb->getMyPid();
  if (master->Test(i) && pcbRecord[i] != NULL) {
      pcbRecord[i]->loneSurvior(t);
  }
  (void) interrupt->SetLevel(oldLevel);
}

void
ProcessManager::addFile(int i, char * name)
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    int sysId = fileManager->searchByName(name);
    UserOpenFile * candidate = NULL;
    
    if  (sysId != -1) 
      candidate = fileManager->track(sysId);
    
    if (master->Test(i) && pcbRecord[i] != NULL
	&& candidate != NULL ) 
      pcbRecord[i]->registerFile(candidate);
    
    (void) interrupt->SetLevel(oldLevel);
}

OpenFile *
ProcessManager::getOpenFile(int i, int fileId)
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    UserOpenFile * candidate = NULL;
    int sysId = -1;
    
    if (master->Test(i) && pcbRecord[i] != NULL)
      if (pcbRecord[i]->getUserFile(fileId) != NULL) {
	candidate = pcbRecord[i]->getUserFile(fileId);
	
	
	sysId = fileManager
	  ->searchByName(candidate->getName()) ;
	
	(void) interrupt->SetLevel(oldLevel);
	return fileManager->getOpenFilePtr(sysId);
      }
    
    (void) interrupt->SetLevel(oldLevel);
    return NULL;
}

int
ProcessManager::rmFile(int i, int fileId)
{
  int result = -1;
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  if (master->Test(i) && pcbRecord[i] != NULL) {
    bool success = pcbRecord[i]->removeFile(fileId);
    DEBUG('a', "File was %s removed\n"
	  , (success) ? "SUCCESSFUL" : "UNSUCCESSFUL");
    if (success)
      result = 1;
  }
  
  (void) interrupt->SetLevel(oldLevel);
  return result;
}
