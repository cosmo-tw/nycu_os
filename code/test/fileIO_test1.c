#include "syscall.h"
#include "stdio.h"

int main(void)
{
	char test[] = "abcdefghijklmnopqrstuvwxyz";
	int success = Create("file1.test");
	OpenFileId fid;
	int i;
	if (success != 1) MSG("Failed on creating file");
	fid = Open("file1.test");
	printf("fid = %d\n",fid);
	if (fid <= 0) MSG("Failed on opening file");
	for (i = 0; i < 26; ++i) {
		MSG("Start to Write!");
		int count = Write(test + i, 1, fid);
		if (count != 1) MSG("Failed on writing file");
	}
	success = Close(fid);
	if (success != 1) MSG("Failed on closing file");
	Halt();
}

