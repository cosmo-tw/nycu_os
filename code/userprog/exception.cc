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
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
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
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	is in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int type = kernel->machine->ReadRegister(2);
	int val;
    int status, exit, threadID, programID;
	DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");
    switch (which) {
    case SyscallException:
      	switch(type) {
      	case SC_Halt:
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
			SysHalt();
                        cout<<"in exception\n";
			ASSERTNOTREACHED();
			break;
		case SC_PrintInt:
		
			SysPrintInt((int)kernel->machine->ReadRegister(4));
			
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
			return;
			ASSERTNOTREACHED();
            break;
		case SC_MSG:
			DEBUG(dbgSys, "Message received.\n");
			val = kernel->machine->ReadRegister(4);
			{
			char *msg = &(kernel->machine->mainMemory[val]);
			cout << msg << endl;
			}
			SysHalt();
			ASSERTNOTREACHED();
			break;
		case SC_Create:
			val = kernel->machine->ReadRegister(4);
			{
			char *filename = &(kernel->machine->mainMemory[val]);
			//cout << filename << endl;
			status = SysCreate(filename);
			kernel->machine->WriteRegister(2, (int) status);
			}
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
			return;
			ASSERTNOTREACHED();
            break;
      	case SC_Add:
			DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");
			/* Process SysAdd Systemcall*/
			int result;
			result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
			/* int op2 */(int)kernel->machine->ReadRegister(5));
			DEBUG(dbgSys, "Add returning with " << result << "\n");
			/* Prepare Result */
			kernel->machine->WriteRegister(2, (int)result);	
			/* Modify return point */
			{
	  		/* set previous programm counter (debugging only)*/
	  		kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
	  			
			/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	  		kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
	  
	 		/* set next programm counter for brach execution */
	 		kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
			}
			cout << "result is " << result << "\n";	
			return;	
			ASSERTNOTREACHED();
            break;

		case SC_Open:
		{
			cout<< "SC_Open is work" <<endl;
			val = kernel->machine->ReadRegister(4);
			char *filename = &(kernel->machine->mainMemory[val]);
			status = SysOpen(filename);
			kernel->machine->WriteRegister(2, (int) status);

			/* 每個功能結束後都要加的東西，還不確定啥意義 */
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
		}

		case SC_Write:
		{
			cout<< "SC_Write is work" <<endl;

			/* 以下是針對fileIO_test1.c的結果 */

			/* val = 812 */
			val = kernel->machine->ReadRegister(4); 
			cout << "val = " << val << endl;

			/* buffer = file1.test，也就是Create出來的檔案的名字 */
			/* 這裡很奇怪，如果引數是按順序放在Register中，那ReadRegister(4)出來的應該要是test + i的東西 */
			/* 不應該可以做為主記憶體的位置查詢才對，那是id的事情(大概)，理應在Register(6)中取得 */
			char *buffer = &(kernel->machine->mainMemory[val]);
			cout << "buffer = " << buffer << endl;

			/* size = 2021095029 */
			/* 按順序這裡應該要提出 1 ，為Write的第2個引數 */
			/* 用這個值去查找mainMemory是沒東西的*/
			/* 意外：ReadRegister(3)會讀出1*/
			int size    = kernel->machine->ReadRegister(1);
			cout << "size = " << size << endl;

			/* fileID = 121 */
			/* 這裡本該出現 fileID，且值應該要是6(在Open的sta tus結果是6)*/
			/* 用這個值去查找mainMemory是沒東西的*/
			int fileID  = kernel->machine->ReadRegister(6);
			cout << "fileID = " << fileID << endl;

			/* RR = 122 */
			/* 只是來看看第4個引數會出來甚麼東西 */
			/* 用這個值去查找mainMemory會出現亂碼*/
			int RR  = kernel->machine->ReadRegister(7);
			cout << "RR = " << RR << endl;

			status = SysWrite(buffer, size, fileID);
			cout << "status = " << status << endl;


			kernel->machine->WriteRegister(2, (int) status);

			/* 每個功能結束後都要加的東西，還不確定啥意義 */
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
		}

		case SC_Close:
		{ 
			cout<< "SC_Close is work" <<endl;
			int fileID = kernel->machine->ReadRegister(4);
			cout << "fileID = " << fileID << endl;

			status = SysClose(fileID); 
			kernel->machine->WriteRegister(2, (int) status);
				
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg)); 
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
		}

		case SC_Exit:
			DEBUG(dbgAddr, "Program exit\n");
            val=kernel->machine->ReadRegister(4);
            cout << "return value:" << val << endl;
			kernel->currentThread->Finish();
            break;

      	default:
			cerr << "Unexpected system call " << type << "\n";
			break;
		}
		break;
	default:
		cerr << "Unexpected user mode exception " << (int)which << "\n";
		break;
    }
    ASSERTNOTREACHED();
}

