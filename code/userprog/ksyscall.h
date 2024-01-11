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

/* This is new add function for HW Part2-1 */
void SysPrintInt(int number)
{
	kernel->synchConsoleOut->PutInt(number);
}

/* This is new add function for HW Part2-2 */
int SysCreate(char *filename)
{
	// return value
	// 1: success
	// 0: failed
	return kernel->interrupt->CreateFile(filename);
}

int SysOpen(char *name)
{	
	return kernel->fileSystem->OpenAFile(name);
}

int SysWrite(char *buffer, int size, int id)
{
	return kernel->fileSystem->Write(buffer, size, id);
}

int SysRead(char *buffer, int size, int id)
{
  return kernel->fileSystem->Read(buffer, size, id);
}

int SysClose(int id)
{
  return kernel->fileSystem->Close(id);
}

#endif /* ! __USERPROG_KSYSCALL_H__ */
