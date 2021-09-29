#ifndef DEPENDENCIES
#define DEPENDENCIES

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>

#define SIZE 10

struct Memory 
{
	int status;
	uint32_t number;
};

#endif