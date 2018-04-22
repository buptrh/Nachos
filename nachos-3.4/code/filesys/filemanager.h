#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H
#define FILE_ENTRIES 128

#include <copyright.h>
#include "bitmap.h"
#include "sysopenfile.h"
#include "useropenfile.h"
#include "openfile.h"


class FileManager {
 public:
  FileManager();
  ~FileManager();
  int Open(OpenFile * f, char * name);
  int CreateFile(OpenFile * f, char * name);
  void decOrClearFile(int i);
  UserOpenFile * track(int i);
  int getFid();
  int searchByName(char * name);
  bool evalNames(char * n1, char * n2);
  void printArr();
  OpenFile * getOpenFilePtr(int id);
  
 private:
  SysOpenFile ** fileRecord;
  BitMap * master;
};

#endif //FILE_MANAGER_H
