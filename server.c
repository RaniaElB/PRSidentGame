#include "server.h"
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
int rand_range(int upper_limit);

void initMemoireLobby();
void readMemoireLobby();
void destroyMemoireLobby();
void initMemoirePileCartes();
void destroyMemoireCardsPile();
void createPipe(char* message, int pid);
bool array_contains(int* haystack, int needle, int length);
void sigint_handler();

int shmId;
int shmIdCardsPile;
int maxPlayers;
char * seg_ptg;
char * cardsPile;
int nbJoueurs;
struct player arrayPlayer[MAXPLAYERS]; 


int main() {
	signal(SIGINT, sigint_handler);
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
	//distribution des cartes dans des pipes
	deal_cards();
	//initialisation partie
	int i;
	int nbTours = 0;
	//init memoire ptg
	initMemoirePileCartes();
	//init semaphore
	sem_t *semMemCardsPile;
	sem_unlink("/memCardsPile");
	if((semMemCardsPile= sem_open("/memCardsPile", O_CREAT | O_EXCL, S_IRWXU, 1)) == SEM_FAILED)
	{	
		perror("can not open semMemCardsPile");
		exit(-1);
	}
	
	// debut partie
	while(1){
	for (i=0; i < nbJoueurs; i++){ //while ???? là on n'a que N tours de jeu
	printf("TOUR %i\n",nbTours);
		int j;
		for (j=0; j < nbJoueurs; j++){
		if (i == j){
			kill(arrayPlayer[j].pid, SIGUSR1);
		}
		else if (!(nbTours == 0 && i == 0)) {
			kill(arrayPlayer[j].pid, SIGUSR2);}
	}
	sigset_t set;
	
	sigemptyset(&set);
	signal(SIGUSR1,SIG_IGN);
/*	signal(SIGUSR2,SIG_IGN);*/
	sigaddset(&set, SIGUSR1);
	int signal;
	printf("waiting for %s to play... \n", arrayPlayer[i].name);
   	sigwait(&set,&signal);
	printf("%s played!! \n", arrayPlayer[i].name);
	sem_wait(semMemCardsPile);
	printf("pile de cartes :%s\n", cardsPile);
	printf("shmIDcardsPile %d\n", shmIdCardsPile);
	sem_post(semMemCardsPile);
	}
	nbTours++;
	}
	
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

void initMemoirePileCartes(){
	key_t cleSegment;
	CHECK(cleSegment=ftok("/memCardsPile", 1), "fack, can't create key");
	CHECK(shmIdCardsPile=shmget(cleSegment, 200 * sizeof(char), IPC_CREAT | SHM_R | SHM_W), "fack, can't create shm");
	printf("shm ID : %i \n", shmIdCardsPile);
	cardsPile = (char*) shmat(shmId, NULL, 0);
}
void createPipe(char* message, int pid){
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


		write(descripteur_pipe_ecriture,message,CHAR_BUFFER_LENGTH);
		fprintf(stdout,"Serveur - message envoyé ..\n");
		kill(pid,10);	
		printf("signal SIGUSR1 envoyé à %i\n",pid); 
		sleep(2);
		close(descripteur_pipe_ecriture);
    		fprintf(stdout,"Serveur - fermeture du tube nommé '%s'.\n",path_pipe_client);
	    	CHECK(
		    remove(path_pipe_client),
		    "Impossible de supprimer le tube nommé.\n"
		);
    		fprintf(stdout,"Serveur - fichier '%s' supprimé.\n",path_pipe_client);
}

void destroyMemoireLobby(){
	CHECK(shmctl(shmId, IPC_RMID, NULL), "cannot delete the content of the segment");
	CHECK(shmdt(seg_ptg), "cannot detach from the segment");
}

void destroyMemoireCardsPile(){
	CHECK(shmctl(shmIdCardsPile, IPC_RMID, NULL), "cannot delete the content of the segment");
	CHECK(shmdt(cardsPile), "cannot detach from the segment");
}

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
		createPipe(msg, arrayPlayer[player].pid);
		printf("cards dealt : \n");
		printf("%s\n", msg);
	}
}

bool array_contains(int* haystack, int needle, int length) {
	int* array_ptr = haystack;
	for (; (array_ptr - haystack) < length; array_ptr++) {
		if (*array_ptr == needle) {
			return true;
		}
	}
	return false;
}

void sigint_handler(int sig, siginfo_t *si, void* arg)
{
	printf("received sigint\n");
	int i;
	for (i=0; i < nbJoueurs; i++){
	printf("sending sigint to pid : %i\n", arrayPlayer[i].pid);
		kill(arrayPlayer[i].pid, SIGINT);
	}
		destroyMemoireLobby();
		destroyMemoireCardsPile();
		exit(0);
}

int rand_range(int upper_limit) {
	return (int) (( (double) upper_limit / RAND_MAX) * rand());
}

