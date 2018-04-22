#include "sysopenfile.h"
#include "system.h"

SysOpenFile::SysOpenFile()
{
  oFile = NULL;
  fId = -1;
  processCount = 0;
}

SysOpenFile::SysOpenFile(OpenFile * f, int id, char * name)
{
  oFile = f;
  fId = id;
  bcopy(name, fName, 255);
  processCount = 0;
}

SysOpenFile::~SysOpenFile()
{
  if (processCount != 0)
    DEBUG('a', "FILE DELETED WHILE OTHERS WERE USING IT\n");

  
  //  fileSystem->Remove(fName); // Remove the file from the fs
  DEBUG('a', "Removal from filesys success... I hope...\n");
  //  printf("Removal from filesys success... I hope...\n");
  fId = -1;
  processCount = 0;
  delete oFile;
  delete fName;
}

void
SysOpenFile::RegisterAttributes(OpenFile * f, int id, char * name)
{
  oFile = f;
  fId = id;
  bcopy(name, fName, 255);
}

int
SysOpenFile::getId()
{
  return fId;
}

char *
SysOpenFile::getName()
{
  return fName;
}

int
SysOpenFile::numReaders()
{
  return processCount;
}

OpenFile *
SysOpenFile::getFilePtr()
{
  return oFile;
}

// ONLY CALL WHEN INTERRUPTS ARE OFF!
void
SysOpenFile::decrementReaders()
{
  processCount--;
}

void
SysOpenFile::incrementReaders()
{
  processCount++;
}
/*
bool
SysOpenFile::ReadFile()
{
  if (oFile->Length() <= 0)
    return false;
  
  processCount++;
  

  return false;
}
*/
