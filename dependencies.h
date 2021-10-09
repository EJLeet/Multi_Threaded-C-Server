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

#define SIZE 10

struct Memory 
{
	int status, s_flag[SIZE], c_flag, complete_threads[SIZE];
	uint32_t number, slot[SIZE];
/*
client_flag = 1 when there is a request outstanding
server_flag = 0 server is active
server_flag = 1 client can read data
server_flag = 2 slot is unused
complete_threads = -1 slot is free
*/
};

#endif