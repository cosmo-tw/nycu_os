/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__ 
#define __USERPROG_KSYSCALL_H__ 

#include "kernel.h"

#include "synchconsole.h"


void SysHalt()
{
  kernel->interrupt->Halt();
}

int SysAdd(int op1, int op2)
{
  return op1 + op2;
}

int SysCreate(char *filename)
{
	// return value
	// 1: success
	// 0: failed
	return kernel->interrupt->CreateFile(filename);
}

/* This is new add function for HW Part2-1 */
void SysPrintInt(int number)
{
	kernel->interrupt->PrintInt(number);
}

/* This is new add function for HW Part2-2 */

// Open a file with the name, and returns its corresponding OpenFileId. 
// Return -1 if fail to open the file.
OpenFileId SysOpen(char *name)
{	
	cout<< "ksyscall.h is work" <<endl;
	return kernel->OpenAFile(name);
}

// int SysWrite(char *buffer, int size, OpenFileId id)
// {
// 	return kernel->fileSystem->WriteAFile(buffer, size, id);
// }

// int SysRead(char *buffer, int size, OpenFileId id)
// {
//   return kernel->fileSystem->ReadAFile(buffer, size, id);
// }

// int SysClose(OpenFileId id)
// {
//   return kernel->fileSystem->CloseAFile(id);
// }

#endif /* ! __USERPROG_KSYSCALL_H__ */
