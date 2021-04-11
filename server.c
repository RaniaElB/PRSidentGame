#include "server.h"
#include "cards.h"
#include <arpa/inet.h>
#include <netdb.h> /* getprotobyname */
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


#define MAXPLAYERS 2 //todo modify everywhere in order to make it dynamic
#define MESSAGE_SIZE 82
#define DEAL		4
#define PATH_PIPE "./fifo-serveur-"
#define CHAR_BUFFER_LENGTH 100

void initMemoireLobby();
void readMemoireLobby();
void destroyMemoireLobby();
void createPipe();

int shmId;
int maxPlayers;
char * seg_ptg;
int nbJoueurs;
struct player arrayPlayer[MAXPLAYERS]; 


int main() {
/*	printf("enter nb of players:\n");*/  //todo modify everywhere in order to make it dynamic
/*	scanf("%i", &maxPlayers);*/
	printf("%i players:\n", MAXPLAYERS);
	initMemoireLobby();
	sem_t *semMemLobby;
	sem_unlink("/memLobby");
	if((semMemLobby= sem_open("/memLobby", O_CREAT | O_EXCL, S_IRWXU, 1)) == SEM_FAILED)
	{	
		perror("can not open semMemLobby");
		exit(-1);
	}
	nbJoueurs = 0;
	while ( nbJoueurs < MAXPLAYERS ){
	printf("Players : %i/%i\n", nbJoueurs, MAXPLAYERS);
	printf("Waiting for semaphore...\n");
	sem_wait(semMemLobby);
	readMemoireLobby();
	sem_post(semMemLobby);
	printf("Gave semaphore back, next check in 1 sec\n\n");
	sleep(1);
	} 
	printf("players : %i/%i, starting game.\n", nbJoueurs, MAXPLAYERS);
	sem_unlink("memLobby");
	destroyMemoireLobby();

/*	int descripteur_pipe_ecriture = 0;*/
/*		CHECK(*/
/*            mkfifo(PATH_PIPE,0600),*/
/*            "Impossible de créer un tube nommé.\n"*/
/*		);*/
/*		fprintf(stdout,"Serveur - tube nommé créé.\n");*/
/*		*/
/*		CHECK(*/
/*				descripteur_pipe_ecriture = open(PATH_PIPE,O_RDWR | O_NONBLOCK),*/
/*				// Remarque : l'appel de open() avec l'attribut WR_ONLY est bloquant sans l'attribut O_NONBLOCK*/
/*				// Il faut que le tube nommé soit ouvert en lecture avant d'être ouvert en écriture.*/
/*				// Lorsque ce n'est pas le cas, et qu'on ajoute l'attribut O_NONBLOCK, on obtient l'erreur "No such device or address"*/
/*				"Impossible d'ouvrir le tube nommé.\n"*/
/*				);*/

/*		char message[CHAR_BUFFER_LENGTH] = "En attente d'un message ..";*/

/*		write(descripteur_pipe_ecriture,message,CHAR_BUFFER_LENGTH);*/
/*		fprintf(stdout,"Serveur - message envoyé ..\n");*/
		int i;
		for (i=0; i < MAXPLAYERS; i++){
		createPipe(arrayPlayer[i].pid);
		}	
	
	
    sleep(10);
    for (i=0; i < MAXPLAYERS; i++){
		
		}

/*    close(descripteur_pipe_ecriture);*/
/*    fprintf(stdout,"Serveur - fermeture du tube nommé '%s'.\n",PATH_PIPE);*/

/*    CHECK(*/
/*            remove(PATH_PIPE),*/
/*            "Impossible de supprimer le tube nommé.\n"*/
/*    );*/
/*    fprintf(stdout,"Serveur - fichier '%s' supprimé.\n",PATH_PIPE);*/
    return EXIT_SUCCESS;
}

void initMemoireLobby(){
	key_t cleSegment;
	CHECK(cleSegment=ftok("/memLobby", 1), "fack, can't create key");
	CHECK(shmId=shmget(cleSegment, 200 * sizeof(char), IPC_CREAT | SHM_R | SHM_W), "fack, can't create shm");
	printf("shm ID : %i \n", shmId);
	seg_ptg = (char*) shmat(shmId, NULL, 0);
}
void readMemoireLobby()
{
printf("seg : %s\n", seg_ptg);
char *tokenAll = strtok(seg_ptg,";");
while (tokenAll != NULL) {
	struct player player_i;
	char *tokenName = strtok(tokenAll,":"); //todo modify in order to handle multiple players to be added at the same time
	//because strtok modifies initial string
	int tokenPID =  atoi(strtok(NULL, ";"));
	player_i.name = malloc(sizeof(char)*10);
	strcpy(player_i.name, tokenName);
	strcat(player_i.name, "\0");
	player_i.pid =  tokenPID;
	printf("joueur %i, name:%s, pid:%i\n", nbJoueurs+1, player_i.name, player_i.pid );
	tokenAll = strtok(NULL, ";");
	arrayPlayer[nbJoueurs] = player_i;
	nbJoueurs++;
}
printf("Current players in the lobby:\n");
size_t i;
for (i = 0; i < nbJoueurs; i++){
printf("joueur %lu name: %s, pid:%i\n", i+1, arrayPlayer[i].name, arrayPlayer[i].pid);
}
strcpy(seg_ptg, "\0");


}

void createPipe(int pid){
int descripteur_pipe_ecriture = 0;
char path_pipe_client[28];
snprintf(path_pipe_client, sizeof(path_pipe_client)+sizeof(pid), "%s%i", PATH_PIPE, pid);
printf("ppc : %s\n", path_pipe_client);
		CHECK(
            mkfifo(path_pipe_client,0600),
            "Impossible de créer un tube nommé.\n"
		);
		fprintf(stdout,"Serveur - tube nommé créé.\n");
		
		CHECK(
				descripteur_pipe_ecriture = open(path_pipe_client,O_RDWR | O_NONBLOCK),
				// Remarque : l'appel de open() avec l'attribut WR_ONLY est bloquant sans l'attribut O_NONBLOCK
				// Il faut que le tube nommé soit ouvert en lecture avant d'être ouvert en écriture.
				// Lorsque ce n'est pas le cas, et qu'on ajoute l'attribut O_NONBLOCK, on obtient l'erreur "No such device or address"
				"Impossible d'ouvrir le tube nommé.\n"
				);

		char message[CHAR_BUFFER_LENGTH] = "Cartes joueur i ";

		write(descripteur_pipe_ecriture,message,CHAR_BUFFER_LENGTH);
		fprintf(stdout,"Serveur - message envoyé ..\n");
		
		kill(pid,10);	
		printf("signal SIGUSR1 envoyé à %i\n",pid); 
		sleep(1);
		close(descripteur_pipe_ecriture);
    		fprintf(stdout,"Serveur - fermeture du tube nommé '%s'.\n",PATH_PIPE);
}

void destroyMemoireLobby(){
	CHECK(shmctl(shmId, IPC_RMID, NULL), "cannot delete the content of the segment");
	CHECK(shmdt(seg_ptg), "cannot detach from the segment");
}

/*
void deal_cards() {
	int cards_per_player = DECK_SIZE / nbJoueurs;
	int dealt_cards[cards_per_player * nbJoueurs];
	int total_dealt_cards = 0;
	int player;
	for (player = 0; player < nbJoueurs; player++) {
		int card;
		int str_length = 0;
		char msg[3* cards_per_player]; //2 caractèress par carte + 1 espace
		msg[0] = '\0';
		for (card = 0; card < cards_per_player; card++) {
			int random_card;
			do {
				random_card = rand_range(DECK_SIZE);
				//choisir une carte tant qu'on n'en trouve pas une qui n'a pas encore été choisie
			} while (array_contains(dealt_cards, random_card, total_dealt_cards));
			//rajouter la carte au message
			str_length += sprintf(msg+str_length, "%d ", random_card);
			dealt_cards[total_dealt_cards++] = random_card;
		}
		//distribuer les cartes choisies au joueur

		printf("cards dealt : \n");
		printf("%s\n", msg);
	}
}
*/
