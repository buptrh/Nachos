// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "mysyscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

#define INC_32 4
#define REG_INVALID -1 // Numerical value outside of int range


void
incProgCounter()
{
  int RegDebugPrev = REG_INVALID;
  int RegDebugCurr = REG_INVALID;
  int RegDebugNext = REG_INVALID;

  char * b = "!!!---- BEGIN REG VALUES ----";
  char * e = "----- END REG VALUES -----";
  
  RegDebugPrev =  machine->ReadRegister(PrevPCReg);
  RegDebugCurr =  machine->ReadRegister(PCReg);
  RegDebugNext =  machine->ReadRegister(NextPCReg);

  DEBUG ('a', "%s\nPrevPCReg: %d\nPCReg: %d\nNextPCReg: %d\n%s\n"
	 ,b, RegDebugPrev, RegDebugCurr, RegDebugNext, e);
  
  machine->WriteRegister(PrevPCReg, RegDebugCurr);
  machine->WriteRegister(PCReg, RegDebugNext);
  machine->WriteRegister(NextPCReg, (RegDebugNext + INC_32));


  RegDebugPrev =  machine->ReadRegister(PrevPCReg);
  RegDebugCurr =  machine->ReadRegister(PCReg);
  RegDebugNext =  machine->ReadRegister(NextPCReg);
  
  DEBUG ('a', "%s\nPrevPCReg: %d\nPCReg: %d\nNextPCReg: %d\n%s\n"
   ,b, RegDebugNext, RegDebugCurr, RegDebugNext, e);
}

void
decProgCounter()
{
  int RegDebugPrev = REG_INVALID;
  int RegDebugCurr = REG_INVALID;
  int RegDebugNext = REG_INVALID;
  
  RegDebugPrev =  machine->ReadRegister(PrevPCReg);
  RegDebugCurr =  machine->ReadRegister(PCReg);
  RegDebugNext =  machine->ReadRegister(NextPCReg);

  machine->WriteRegister(PrevPCReg, PrevPCReg - INC_32);
  machine->WriteRegister(PCReg, PCReg - INC_32);
  machine->WriteRegister(NextPCReg, (NextPCReg - INC_32));
}

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
    DEBUG('a', "ExceptionHandler entry point has been hit. Register value: %d\n"
	  , type);
    if(which == SyscallException) {

      switch (type) {
      case SC_Exit : 
        DEBUG('a', "SC_Exit stub direct hit cmdr!\n");
        MyExit();
        break;
      case SC_Exec : 
        DEBUG('a', "SC_Exec stub direct hit cmdr!\n");
	MyExec();
	break;
      case SC_Join : 
        DEBUG('a', "SC_Join stub direct hit cmdr!\n");
	MyJoin();
        break;
      case SC_Create : 
        DEBUG('a', "SC_Create stub direct hit cmdr!\n");
        MyCreate();
        break;
      case SC_Open : 
        DEBUG('a', "SC_Open stub direct hit cmdr!\n");
        MyOpen();
        break;
      case SC_Write : 
        DEBUG('a', "SC_Write stub direct hit cmdr!\n");
        MyWrite();
        break;
      case SC_Read :
        DEBUG('a', "SC_Read stub direct hit cmdr!\n");
        MyRead();
        break;
      case SC_Close :
        DEBUG('a', "SC_Close stub direct hit cmdr!\n");
        MyClose();
        break;
      case SC_Fork : 
        DEBUG('a', "SC_Fork stub direct hit cmdr!\n");
        MyFork();
        break;
      case SC_Yield : 
        DEBUG('a', "SC_Yield stub direct hit cmdr!\n");
	MyYield();
        break;
      case SC_Kill:
	DEBUG('a', "SC_Kill stub direct hit cmdr!\n");
	MyKill();
	break;
      case SC_Halt:
        DEBUG('a', "Shutdown, initiated by user program.\n");
        interrupt->Halt();
        break;
      default: 
        break;
      }
      
      incProgCounter();
    } else if (which == PageFaultException) {
      /* Stage 3: */
      /* Save the faulting virtual address in reg to be used by 
	 kernel to handle the exception */
      /* This is done in RaiseException BadVAddrReg */
      int vAddr = machine->ReadRegister(BadVAddrReg);
      DEBUG('a', "PageFaultException, begin to rectify fault!\n");
      // //      printf("PageFaultException, begin to rectify fault!\n");
      // printf("Addrspace %s caused the fault!\n", currentThread->space->name);
      printf("++++++++++++++ BadVAddr was %d\n", vAddr);
      
      // OpenFile * swapFile = NULL;
      // swapFile = fileSystem->Open(currentThread->space->name);
      // if (swapFile != NULL) {

      // }
      currentThread->space->sendToMem(vAddr);

      //      decProgCounter();
    } else {
      printf("Unexpected user mode exception %d %d\n", which, type);
      ASSERT(FALSE);
    }
}
