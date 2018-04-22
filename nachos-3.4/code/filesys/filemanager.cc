#include "filemanager.h"
#include <system.h>

FileManager::FileManager()
{
  fileRecord = new SysOpenFile*[FILE_ENTRIES];
  master = new BitMap(FILE_ENTRIES);
  master->Mark(0);
  master->Mark(1);
}

FileManager::~FileManager()
{
  delete master;
  delete [] fileRecord;
}

int
FileManager::Open(OpenFile * f, char * name)
{
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  int tempId = searchByName(name);
  //printf("TempId at this start is %d\n", tempId);
  if (tempId == -1) {
    tempId = CreateFile(f, name);
    //printf("TempId at this point is %d\n", tempId);
    if (tempId == -1) {
      (void) interrupt->SetLevel(oldLevel);
      return -1;
    }
  }


  //printf("TempId at this end  is %d\n", tempId);

  int pid = currentThread->space->pcb->getMyPid();
  DEBUG('a', "Trying to add file with id %d to process %d\n"
	 , tempId, pid);
  processManager->addFile(pid, name);
  (void) interrupt->SetLevel(oldLevel);

  return tempId;
}

int
FileManager::CreateFile(OpenFile * f, char * name)
{
  int candidateFileId = getFid();
  if (candidateFileId == -1 || f == NULL || name == NULL)
    return -1;

  SysOpenFile * sysFile = new SysOpenFile(f, candidateFileId, name);
  fileRecord[candidateFileId] = sysFile;


  return candidateFileId;
}

int
FileManager::getFid()
{
  int candidateFileId = -1;
  candidateFileId = master->Find();
  // printf("found slot at %d\n", candidateFileId);
  return candidateFileId;
}

void
FileManager::decOrClearFile(int i)
{
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  if (master->Test(i) && fileRecord[i] != NULL) {
    //    printf("This will decrement the file readers!\n");
    //    printf("Before readeres is %d\n", fileRecord[i]->numReaders());
    fileRecord[i]->decrementReaders();
    //    printf("After readeres is %d\n", fileRecord[i]->numReaders());
    if (fileRecord[i]->numReaders() == 0) {
      ASSERT (fileRecord[i]->numReaders() == 0);
      //      printf("This will delete the file completely!\n");
      master->Clear(i);
      delete fileRecord[i];
      //      fileRecord[i] = NULL;
    }
  }
  (void) interrupt->SetLevel(oldLevel);
}

UserOpenFile *
FileManager::track(int i)
{
  UserOpenFile * usr = NULL;
  
  if (master->Test(i) && fileRecord[i] != NULL) {
    fileRecord[i]->incrementReaders();
    usr = new UserOpenFile(fileRecord[i]->getName(), 0, -1);
    return usr;
  }
  
  return NULL;
}

int
FileManager::searchByName(char * name)
{
  int i;

  //  printf("searching for %s\n", name);
  for(i = 0 ; i < FILE_ENTRIES ; i++) {
    if (fileRecord[i] != NULL) {
      DEBUG('a', "Evaluating %s and %s\n"
	    , name, fileRecord[i]->getName());
      if ( evalNames(name, fileRecord[i]->getName()) ) 
	return fileRecord[i]->getId();
    }
  }
  return -1;
}

bool
FileManager::evalNames(char * n1, char * n2)
{
  int i;
  for (i = 0 ; true ; i++) {
    DEBUG('a', "Left char is  %c  %s Right Char %c\n"
	  , n1[i], (n1[i] == n2[i]) ? "=" : "!="
	  ,n2[i]);
    if (n1[i] != n2[i])
      return false;
    if (n1[i] == '\0')
      return true;
    if (i > 255)
      return false; // something has gone very wrong if we get here
  } 
}

void
FileManager::printArr()
{
  //  printf("------ PRINTARR CALLED\n");
  int i;
  for(i = 0 ; i < FILE_ENTRIES ; i++) {
    if (fileRecord[i] != NULL) {
      DEBUG('a', "fileRecord %d is named %s\n"
	     , i, fileRecord[i]->getName());
      //      printf("fileRecord %d is named %s\n"
      //	     , i, fileRecord[i]->getName());
    }
  }
}

OpenFile *
FileManager::getOpenFilePtr(int id)
{
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  if (master->Test(id) && fileRecord[id] != NULL) {
    (void) interrupt->SetLevel(oldLevel);
    return fileRecord[id]->getFilePtr();
  }
  // Actually on second thought this may not need to be exclusive
  (void) interrupt->SetLevel(oldLevel);
  return NULL;
}
 
