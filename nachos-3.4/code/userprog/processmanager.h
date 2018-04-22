#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H
#define PCB_ENTRIES 32

#include <copyright.h>
#include <bitmap.h>
#include <pcb.h>
#include "thread.h"
#include "filemanager.h"
#include "sysopenfile.h"

class ProcessManager {
 public:
  ProcessManager();
  ~ProcessManager();
  int getPid(int parentPid);
  void clearPid(int i);
  PCB* createPCB(int parentPid, Thread * t, bool forked);
  PCB* getPCB(int i);
  void addThread(int pid, Thread * t);
  void endThread(int pid, Thread * t);
  bool hasMultiThreads(int i);
  void endProcess(int i);
  void endOtherThreads(Thread * t);
  void addFile(int i, char * name);
  OpenFile * getOpenFile(int i, int fileId);
  int rmFile(int i, int fileId);
  
 private:
  PCB ** pcbRecord;
  BitMap * master;
};

#endif //PROCESS_MANAGER_H
