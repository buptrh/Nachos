#include "pcb.h"
#include "system.h"
#include "syscall.h"
#define OPEN_FILE_LIMIT 32

PCB::PCB(int gPid, int gParentPid,  void * tPtr)
{
  pid = gPid;
  parentPid = gParentPid;
  myThreads = new List;
  myThreads->Append(tPtr);
  children = new List;
  myFiles = new UserOpenFile*[OPEN_FILE_LIMIT];
  master = new BitMap(OPEN_FILE_LIMIT);

  /* Special files that represent stdin & stdout */
  master->Mark(ConsoleInput);
  master->Mark(ConsoleOutput);
}

PCB::~PCB()
{
  
  pid = -1;
  parentPid = -1;
  delete myThreads;
  delete children;

  /* New deletes proj4 */
  //delete myFiles;
  //delete master;
}

int
PCB::getMyPid()
{
  return pid;
}

int
PCB::getParent()
{
  return parentPid;
}

void
PCB::invalidateParent()
{
  parentPid = -1;
}

static void
setParent(int childPid)
{
  ASSERT(processManager->getPCB(childPid) != NULL);
  processManager->getPCB(childPid)->invalidateParent();
}

void
PCB::removeParent()
{
  children->Mapcar(setParent);
}

bool
PCB::hasChildren()
{
  return !children->IsEmpty();
}

void
PCB::removeChild(int childPid)
{
  int i = (int) children->RemoveByKey(childPid);
  DEBUG('a', "Child removed %d\n", i);
}

void
PCB::registerChild(int childPid)
{
  children->Append((void *) childPid); 
}

void
PCB::registerThread(void * t)
{
  myThreads->Append(t); 
}

void
PCB::removeThread(void * t)
{
  int addr = (int) t;
  myThreads->RemoveByKey(addr); 
}

bool
PCB::isMultiThreaded()
{
  return !myThreads->isSingleton();
}

bool
PCB::hasChild(int i)
{
  if (!hasChildren())
    printf("No children\n");
  
  return children->SearchByKey(i);
}

static void
killAllThreads(int tAddr)
{
  Thread * t = (Thread *) tAddr;
  int p = t->space->pcb->getMyPid();
  processManager->endThread(p, t);
  delete t->space;
  if (t == currentThread)
    t->Finish();
  else
    scheduler->ReadyToDone(t);
}
static void
listThread(int tAddr)
{
  Thread * t = (Thread *) tAddr;
  
  printf("I am a thread in the list my name is %s and pid is %d\n"
	 , t->getName(), t->space->pcb->getMyPid());
}

void
PCB::loneSurvior(void * t)
{
  printf("This is the thread %d\n", (int) t);
  

  printf("Mapcar is not breoken emp  %s single %s\n",
	 myThreads->IsEmpty() ? "true" : " false",
	 myThreads->isSingleton() ? "true" : " false");

  myThreads->Mapcar(listThread);  
  
  List* removed = myThreads->RemoveAllButKey((int)t);
    printf("RMRMRMRM emp  %s single %s\n",
	 removed->IsEmpty() ? "true" : " false",
	 removed->isSingleton() ? "true" : " false");
  removed->Mapcar(listThread);
  removed->Mapcar(killAllThreads);
  delete removed;
}

void
PCB::listAllThreads()
{
  printf("----Ive  been called\n");
  myThreads->Mapcar(listThread);
}

void
PCB::suicide()
{ 
  myThreads->Mapcar(killAllThreads);
}

void
PCB::registerFile(UserOpenFile * usrFile)
{
  int i = master->Find();
  if (i == -1)
    return;
  usrFile->setFileId(i);
  myFiles[i] = usrFile;
  DEBUG('a', "registered file %s in pcb of process %d\n"
	 , myFiles[i]->getName(), pid);
}

bool
PCB::removeFile(int i)
{
  if( master->Test(i) && myFiles[i] != NULL) {
    DEBUG('a', "Removed file %s with fileId %d\n"
	 ,myFiles[i]->getName(), myFiles[i]->getFileId()); 
    master->Clear(i);
    delete myFiles[i];
    return true;
  }

  return false;
}

UserOpenFile *
PCB::getUserFile(int i)
{
  if( master->Test(i) && myFiles[i] != NULL) 
      return myFiles[i];

  return NULL;
}

UserOpenFile *
PCB::retrieveFile(char * name)
{
  int i;
  for (i = 0 ; i < OPEN_FILE_LIMIT ; i++) {
    if (myFiles[i] != NULL)
      if ( evalNames(name, myFiles[i]->getName()) )
	return myFiles[i];

  }
  return NULL;
}

bool
PCB::evalNames(char * n1, char * n2)
{
  int i;
  for (i = 0 ; true ; i++) {
    //    printf("n1 %c ---- n2 %c \n", n1, n2);
    if (n1[i] != n2[i])
      return false;
    if (n1[i] == '\0')
      return true;
    if (i > 255)
      return false; // something has gone very wrong if we get here
  } 
}

void
PCB::printMyFiles()
{
  //  printf("++++ PRINTING MYFILES CALLED\n");
  int i;
  for(i = 0 ; i < OPEN_FILE_LIMIT ; i++) {
    if (myFiles[i] != NULL) {
      DEBUG('a',"myFiles[%d] is named %s\n"
	     , i, myFiles[i]->getName());
    }
  }
}

/*
void *
PCB::getThreadPtr()
{
  return t;
  }*/
