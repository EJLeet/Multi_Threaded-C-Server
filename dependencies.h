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
#include <semaphore.h>
#include <time.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>

#define SIZE 10

struct Memory 
{
	int s_flag[SIZE], c_flag, progress[SIZE];
	uint32_t number, slot[SIZE];
};
/*
c_flag = 1 when there is a request outstanding
c_flag = 8 test mode
c_flag = 9 server can quit

s_flag = 0 server is active
s_flag = 1 client can read data
s_flag = 2 slot is unused

progress = -1 slot is free
*/

#endif