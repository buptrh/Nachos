#ifndef PCB_H
#define PCB_H

#include <copyright.h>
//#include "thread.h"
#include "list.h"
#include "sysopenfile.h"
#include "filemanager.h"
#include "bitmap.h"
#include "useropenfile.h"

class PCB {
 public:
  PCB(int gPid, int gParentPid, void * tPtr);
  ~PCB();
  int getMyPid();
  int getParent();
  int getChild(int childPid);
  void invalidateParent();
  void removeParent();
  bool hasChildren();
  void removeChild(int childPid);
  void registerChild(int childPid);
  bool isMultiThreaded();
  void * getThreadPtr();
  void registerThread(void * t);
  void removeThread(void *t);
  void suicide();
  void listAllThreads();
  bool hasChild(int i);
  void loneSurvior(void * t);
  void registerFile(UserOpenFile * usrFile);
  UserOpenFile *  getUserFile(int i);
  UserOpenFile * retrieveFile(char * name);
  bool evalNames(char * n1, char * n2);
  bool removeFile(int i);
  void printMyFiles();
  
 private:
  int pid;
  int parentPid;
  List * myThreads;
  List * children;
  UserOpenFile ** myFiles;
  BitMap * master;
};

#endif //PCB_H

