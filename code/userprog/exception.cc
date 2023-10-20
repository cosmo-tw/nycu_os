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
			
			/* r4~r7 是存放函式的引數(按順序)，現在 r4 存的是建立的檔案的名稱在主記憶體的位置 */
			val = kernel->machine->ReadRegister(4);
			char *filename = &(kernel->machine->mainMemory[val]);

			/* status 存放的是 file 的ID */
			status = SysOpen(filename);
			// cout<< "open file ID = " << status << endl;

			kernel->machine->WriteRegister(2, (int) status);

			/* 每個功能結束後都要加的東西，還不確定啥意義 */
			/* return可能跟釋放register有關，沒加的話會導致使用完這個syscall之後，用其他的syscall沒辦法正確傳遞引數 */
			/* 會使其他syscall這裡提取 r4~r7 時還是會拿到上一個syscall的引數資料 */
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
			return;
			ASSERTNOTREACHED();
            break;
		}

		case SC_Write:
		{
			/* 這裡拿到的就是要寫入的內容在主記憶體上的位置 */
			/* 指向要寫的內容的位置 */
			val = kernel->machine->ReadRegister(4); 
			// cout << "val = " << val << endl;

			/* 這邊的buffer不太確定為什麼是存取a~z，下一次是b~z，而不是一個一個寫 */
			char *buffer = &(kernel->machine->mainMemory[val]);
			// cout << "buffer = " << buffer << endl;

			/* 要寫入的大小，這是用來對答案的，如果寫入的大小與輸入的大小對不上就會報錯 */
			int size    = kernel->machine->ReadRegister(5);

			/* file ID，理論上要跟 Open() 所返還的值是一樣的 */
			int fileID  = kernel->machine->ReadRegister(6);
			// cout << "fileID = " << fileID << endl;

			/* status存放實際上寫了多少個char，如果嘗試寫入非法的file理應返回-1 */
			status = SysWrite(buffer, size, fileID);
			// cout << "status = " << status << endl;
			kernel->machine->WriteRegister(2, (int) status);

			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
			return;
			ASSERTNOTREACHED();
            break;
		}

		case SC_Read:
		{
			/* 這裡拿到的就是要寫入的內容在主記憶體上的位置 */
			/* 指向要讀的內容的位置 */
			val = kernel->machine->ReadRegister(4); 
			// cout << "val = " << val << endl;

			/* 這邊的buffer不太確定為什麼是存取a~z，下一次是b~z，而不是一個一個寫 */
			char *buffer = &(kernel->machine->mainMemory[val]);
			// cout << "buffer = " << buffer << endl;

			/* 要寫入的大小，這是用來對答案的，如果讀取的大小與輸入的大小對不上就會報錯 */
			int size    = kernel->machine->ReadRegister(5);

			/* file ID，理論上要跟 Open() 所返還的值是一樣的 */
			int fileID  = kernel->machine->ReadRegister(6);
			// cout << "fileID = " << fileID << endl;

			/* status存放實際上讀了多少個char，如果嘗試讀取非法的file理應返回-1 */
			status = SysRead(buffer, size, fileID);
			// cout << "status = " << status << endl;
			kernel->machine->WriteRegister(2, (int) status);

			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
			return;
			ASSERTNOTREACHED();
            break;
		}

		case SC_Close:
		{ 
			/* 一樣，file ID應跟 Open()的返還值一樣 */
			int fileID = kernel->machine->ReadRegister(4);
			// cout << "close fileID = " << fileID << endl;

			/* status會存 0(關檔失敗)或1(關檔成功) */
			status = SysClose(fileID); 

			// cout<< "SysClose = " << status <<endl;
			kernel->machine->WriteRegister(2, (int) status);
				
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg)); 
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
			return;
			ASSERTNOTREACHED();
            break;
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

