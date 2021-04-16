#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h> /* getprotobyname */
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdbool.h>
#include "cards.c"

#define MAX 10
#define MESSAGE_SIZE 82
#define PATH_PIPE "./fifo-serveur-"
#define CHAR_BUFFER_LENGTH 100

void initMemoireLobby();
void initMemoirePileCartes();
void input();
void sig_handler_empty();
void sigint_handler();
void afficher_cartes();
int decode_msg_payload(char** raw_payload, int* decoded_payload, int max_elements); 
bool basicPlay(int index);
void readCardPile();
void inscriptionJoueur();

#define CHECK(sts, msg) if ((sts) == -1){perror(msg);exit(-1);}

struct player {
	char name[10];
	int *pid;
};
