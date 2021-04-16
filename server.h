#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "cards.h"
#include <arpa/inet.h>
#include <netdb.h> /* getprotobyname */
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>


#define MAXPLAYERS 2 //todo modify everywhere in order to make it dynamic
#define MESSAGE_SIZE 82
#define DEAL		4
#define PATH_PIPE "./fifo-serveur-"
#define CHAR_BUFFER_LENGTH 100

//shared memory
void initMemoireLobby();
void readMemoireLobby();
void destroyMemoireLobby();

void initMemoirePileCartes();
void destroyMemoireCardsPile();
void waitingForPlayers(sem_t* semMemLobby);
//pipe function
void createPipe(char* message, int pid);
//utils functions
bool array_contains(int* haystack, int needle, int length);
int rand_range(int upper_limit);
//signal handling callback functions
void sigint_handler();
void sig_handler_empty();
////


#define CHECK(sts, msg) if ((sts) == -1){perror(msg);exit(-1);}

struct player {
	char *name;
	int pid;
};

void deal_cards();

