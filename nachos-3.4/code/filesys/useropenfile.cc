#include "useropenfile.h"
#include "system.h"

UserOpenFile::UserOpenFile(char * name, int offsetIn, int id)
{
  fileName = name;
  offset = offsetIn;
  fileId = id;
}

UserOpenFile::~UserOpenFile()
{
  //  printf("Now in useropenfile destructor decAndClear!\n");
  int sysId = fileManager->searchByName(fileName);
  fileManager->decOrClearFile(sysId);
  //  delete fileName; The delete will be handled by sysfile
  offset = -1;
  fileId = -1;
}

char *
UserOpenFile::getName()
{
  return fileName;
}

void 
UserOpenFile::setOffset(int i)
{
  offset = i;
}

int
UserOpenFile::getCurOffset()
{
  return offset;
}

int
UserOpenFile::getFileId()
{
  return fileId;
}

void
UserOpenFile::setFileId(int i)
{
  fileId = i;
}

