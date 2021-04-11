#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>


#define CHECK(sts, msg) if ((sts) == -1){perror(msg);exit(-1);}

struct player {
	char *name;
	int pid;
};

