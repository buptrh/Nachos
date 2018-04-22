#ifndef SYSOPENFILE_H
#define SYSOPENFILE_H

#include "openfile.h"

class SysOpenFile {

 public:  
  SysOpenFile();
  SysOpenFile(OpenFile * f, int id, char * name);
  ~SysOpenFile();
  void RegisterAttributes(OpenFile * f, int id, char * name);
  int numReaders();
  void decrementReaders();
  void incrementReaders();
  int getId();
  char * getName();
  OpenFile * getFilePtr();
  
 private:
  OpenFile * oFile;
  int fId;
  char fName[255];
  int processCount;
};
#endif //SYSOPENFILE_H
