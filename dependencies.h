#ifndef DEPENDENCIES
#define DEPENDENCIES

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define  NOT_READY  -1
#define  FILLED     0
#define  TAKEN      1

struct Memory 
{
	int  status;
	int  data[4];
};

#endif