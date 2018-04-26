#include "syscall.h"
#include "system.h"


void ForkThreadToRun(int param) {
  currentThread->RestoreUserState();
  currentThread->space->RestoreState();
  
  //printf("StackReg is %d\n", machine->ReadRegister(StackReg));
  machine->Run();
}
void MyFork()
{
  printf("System Call: [%d] invoked %s\n",
	 currentThread->space->pcb->getMyPid()
	 , "Fork");
  // 1.
  //  return;

  // 2. 3.
  Thread * child =  new Thread("myFork");
  child->space = new AddrSpace(processManager
    ->createPCB(currentThread->space->pcb->
		getMyPid(), child, true));
  currentThread->space->Fork(child->space);

  // if (child->space->GetNumPages() == -1) {
  //   printf("Deleting child \n");
  //   delete child->space;
  //   delete child;
  //   return;
  // }
  
  currentThread->SaveUserState();  

  // 4. 5.
  

  // 6
  int func = machine->ReadRegister(4);
  machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
  machine->WriteRegister(PCReg, func);
  machine->WriteRegister(NextPCReg, func + 4);
  child->SaveUserState();

  printf("Process [%d] Fork: start at address [0x%04X] with [%d] pages memory\n",
	 currentThread->space->pcb->getMyPid(), func, child->space->GetNumPages());
  
  // 7.  
  child->Fork(ForkThreadToRun, 0);

  // 8.
  currentThread->RestoreUserState();

  // 9.
  machine->WriteRegister(2, child->space->pcb->getMyPid());
}



void MyYield() {
  printf("System Call: [%d] invoked %s\n",
  	 currentThread->space->pcb->getMyPid() , "Yield");
  currentThread->Yield();
}

void MyExit() {
  printf("System Call: [%d] invoked %s\n",
	 currentThread->space->pcb->getMyPid()
	 , "Exit");
  int exitCode = machine->ReadRegister(4);
  printf("Process [%d] exits with [%d]\n",
	 currentThread->space->pcb->getMyPid()
	 , exitCode );

  
  //processManager->endProcess(currentThread->space->pcb->getMyPid());
  delete currentThread->space;
  currentThread->Finish();
}

void MyJoin() {
  printf("System Call: [%d] invoked %s\n",
  	 currentThread->space->pcb->getMyPid() , "Join");
  int pid = machine->ReadRegister(4);
  // printf("B4 while\n");
  
  while(currentThread->space->pcb->hasChild(pid)) {
    //printf("We are waiting for pid [%d]\n", pid);
    currentThread->Yield();
  }

  // printf("After while\n");
  
  //todo: how to get exit code

}

void MyKill() {
  printf("System Call: [%d] invoked %s\n",
  	 currentThread->space->pcb->getMyPid() , "Kill");
  int pid = machine->ReadRegister(4);

  if(processManager->getPCB(pid) == NULL) {
    printf("Process [%d] cannont kill process [%d]: doesn't exist\n"
	   ,currentThread->space->pcb->getMyPid(), pid);
    return;
  }
  printf("Process [%d] killed process [%d]\n"
	   ,currentThread->space->pcb->getMyPid(), pid);
  if(pid == currentThread->space->pcb->getMyPid()) {
    MyExit();
    return;
  }
  
  
  //todo: call exit for this process.
  processManager->endProcess(pid);
  machine->WriteRegister(2, 0);
}


void LoadExecFromMem (int addr, int size, int *value, AddrSpace * a)
{
  int data;
  int physAddr = -1;
 
  machine->ReadMem(addr, size, value);
}


void MyExec() {
  
  printf("System Call: [%d] invoked %s\n",
   currentThread->space->pcb->getMyPid()
   , "Exec");
  
  int addr = machine->ReadRegister(4);
  char fileName[255];
  //bzero(fileName, 255);
  
  machine->ReadMem(addr, 1, (int *) (fileName + 100));
  // printf("filename 100 %c\n", fileName[100]);
  int i = 0;
  for (i = 0 ; true ; i++) {
    LoadExecFromMem(addr + i, 1, (int*)(fileName + i),
		    currentThread->space);
    // printf("filename i %d is %c\n", i, fileName[i]);
    if(fileName[i] == '\0') 
      break;
  }

  // printf(">> Successfully arrived here\n");
  
  OpenFile *executable = fileSystem->Open(fileName);
  if(executable == NULL) {
    machine->WriteRegister(2, -1);
    return;
  }

  PCB * pcb = currentThread->space->pcb;
  AddrSpace * oldSpace  = currentThread->space;
  currentThread->space->evictPages();
  currentThread->space->pcb = NULL;
  delete oldSpace;

  currentThread->space = new AddrSpace(executable, false, fileName, pcb);

  //currentThread->space->swapFile = temp;
  printf("Exec Program: [%d] loading [%s]\n",
	 currentThread->space->pcb->getMyPid(), fileName);
  
  currentThread->space->InitRegisters();
  currentThread->space->RestoreState();
  
  machine->Run();
  machine->WriteRegister(2, 1);
  
  
  MyExit();
}

void MyCreate()
{
  printf("System Call: [%d] invoked %s\n",
   currentThread->space->pcb->getMyPid()
   , "Create");
  
  int addr = machine->ReadRegister(4);
  char fileName[255];
  
  int i = 0;
  machine->ReadMem(addr, 1, (int *) (fileName + 100));
  for (i = 0 ; true ; i++) {
    LoadExecFromMem(addr + i, 1, (int*)(fileName + i),
		    currentThread->space);
    if(fileName[i] == '\0') 
      break;
  }
  //printf("FileName is: %s\n", fileName);
  bool success = false;
  success = fileSystem->Create(fileName, 0);

  DEBUG('a', "Creation of file %s was a %s", fileName
	 , success ? "Success!\n" : "Failure!\n");
}

void MyOpen()
{
  printf("System Call: [%d] invoked %s\n",
   currentThread->space->pcb->getMyPid()
   , "Open");

  int addr = machine->ReadRegister(4);
  char fileName[255];

  int i = 0;
  machine->ReadMem(addr, 1, (int *) (fileName + 100));
  for (i = 0 ; true ; i++) {
    LoadExecFromMem(addr + i, 1, (int*)(fileName + i), currentThread->space);
    if(fileName[i] == '\0') 
      break;
  }

  OpenFile *file = fileSystem->Open(fileName);
  if(file == NULL) {
    //    printf("NULL FILE!");
    machine->WriteRegister(2, -1);
    return;
  }
  int sysId = fileManager->Open(file, fileName);
  if (sysId == -1) {
    machine->WriteRegister(2, -1);
    return;
  }
  
  int fileId = (currentThread->space->pcb->retrieveFile(fileName))
    ->getFileId();
  
  if (fileId == -1) {
    machine->WriteRegister(2, -1);
  }  else {
    machine->WriteRegister(2, fileId);
  }

  DEBUG('a', "Opening of file %s was a %s", fileName
	 , fileId != -1 ? "Success!\n" : "Failure!\n");
}

void MyWrite()
{
  printf("System Call: [%d] invoked %s\n",
   currentThread->space->pcb->getMyPid()
   , "Write");

  //  char * buf = (char *) machine->ReadRegister(4);
  int bufAddr = machine->ReadRegister(4);
  int size    = machine->ReadRegister(5);
  int fileId  = machine->ReadRegister(6);
  if (fileId == -1)
    return;

  char * suppliedBuf = new char[size + 1];

  int i = 0;
  machine->ReadMem(bufAddr, 1, (int *) (suppliedBuf + 100));
  for (i = 0 ; true ; i++) {
    LoadExecFromMem(bufAddr + i, 1, (int*)(suppliedBuf + i),
		    currentThread->space);
    if(suppliedBuf[i] == '\0') 
      break;
  }
  

  //  printf("buffAddr is %d, size is %d, fileId is %d\n", bufAddr, size, fileId);
  char * myOwnBuf = new char[size + 1];
  int k = currentThread->space->userReadWrite(myOwnBuf, bufAddr, size, false);
  if (k == -1)
    return;

  //  printf("k is = to %d\n", k);
  
  myOwnBuf[size] = '\0';
  if (fileId == ConsoleOutput) {
    printf("%s\n", myOwnBuf);
    return;
  }
  int pid = currentThread->space->pcb->getMyPid();
  //  printf("Before handle\n");
  OpenFile * handle = processManager->getOpenFile(pid, fileId);
  //  printf("After handle\n");
  if (handle == NULL) {
    DEBUG('a', "Attempt to write to non-existant or unopened file!");
    // printf("Attempt to WRITE non-existant or unopened file!");
    return;
  }
  
  DEBUG('a', "Loading new buf %s With size %d\n", myOwnBuf, size);
  int newOffset = handle->Write(myOwnBuf, size);
  //  printf("WRITE OFFSET is %d\n", newOffset);
}

void MyRead()
{
  printf("System Call: [%d] invoked %s\n",
   currentThread->space->pcb->getMyPid()
   , "Read");

  int bufAddr = machine->ReadRegister(4);
  int size    = machine->ReadRegister(5);
  int fileId  = machine->ReadRegister(6);

  if (fileId == -1) {
    machine->WriteRegister(2, -1);   
    return;
  }
  char * myOwnBuf = new char[size + 1];
  if (fileId == ConsoleInput) {
    int i;
    for (i = 0 ; i < size ; i++)
      myOwnBuf[i] = getchar();

    DEBUG('a', "Read this from consoleInput %s\n"
	   , myOwnBuf);
    int k = currentThread
      ->space->userReadWrite(myOwnBuf, bufAddr, size, true);
    // printf("Contents read are %s\n", myOwnBuf);					
    machine->WriteRegister(2, k);  
    return;
  }
  int pid = currentThread->space->pcb->getMyPid();

  OpenFile * handle = processManager->getOpenFile(pid, fileId);
  if (handle == NULL) {
    DEBUG('a', "Attempt to read from non-existant or unopened file!");
    //printf("Attempt to read from non-existant or unopened file!");
    machine->WriteRegister(2, -1);  
    return;
  }

  int offset = (currentThread->space->pcb->getUserFile(fileId))
    ->getCurOffset();
  int newOffset = handle->ReadAt(myOwnBuf, size, offset);
  if (newOffset == 0) {
    machine->WriteRegister(2, -1);
    return;
  }
  (currentThread->space->pcb->getUserFile(fileId))
    ->setOffset(offset + newOffset);
  //printf("READ OFFSET is %d\n", newOffset);
  DEBUG('a', "Last entry in myOwnBuf is %c\n", myOwnBuf[size]);
  myOwnBuf[size] = '\0'; // Adding NULL BYTE at end of buffer

  DEBUG('a', "Contents read are %s\n", myOwnBuf);

  //printf("Contents read are %s\n", myOwnBuf);

  int ret = currentThread->space->userReadWrite(myOwnBuf, bufAddr, size, true);
  //  printf("+++++=Ret is equal to %d\n", ret);
  machine->WriteRegister(2, ret);  
}

void MyClose()
{
  printf("System Call: [%d] invoked %s\n",
   currentThread->space->pcb->getMyPid()
   , "Close");
  
  int fId = machine->ReadRegister(4);
  if (fId == -1)
    return;

  //  fileManager->printArr();
  //  currentThread->space->pcb->printMyFiles();


  int pid = currentThread->space->pcb->getMyPid();
  int ret = processManager->rmFile(pid, fId);

  DEBUG('a', "Closing of file %d was a %s", fId 
	 , ret != -1 ? "Success!\n" : "Failure!\n");
}
