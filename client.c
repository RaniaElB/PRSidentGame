#include "client.h"
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
void sigusr_handler();
void afficher_cartes();
int decode_msg_payload(char** raw_payload, int* decoded_payload, int max_elements); 
int basicPlay(int index,char * message);
void readCardPile();
int cards[DECK_SIZE/2]; // todo : set la taille de cards un peu mieux
int nbCards;
char *myName;
char * cardsPile;
int playing ;
int shmId; // shmId memLobby
int shmIdCardsPile;
int pidServer;
int cartePrecedente = -1;
int main(int argc,char * argv[]) 
{
printf("my pid: %i\n",getpid());
	initMemoireLobby();
	sem_t *semMemLobby;
	if((semMemLobby= sem_open("/memLobby", O_CREAT, S_IRWXU, 1)) == SEM_FAILED)
	{	
		perror("can not open semMemLobby");
		exit(-1);
	}
	
	myName = (char *) malloc(MAX * sizeof(char));
	printf("Enter your name: ");
	input(myName, 10);
	asprintf(&myName, "%s:%i;", myName,getpid());
	printf("Waiting for sem...\n");
	sem_wait(semMemLobby);
	printf("semaphore available ! writing in shm...\n");
	char * seg_ptg;
	seg_ptg = shmat(shmId, NULL, 0);
	strcat(seg_ptg, myName);
	printf("text from shared memory : \n%s\n", seg_ptg);
	sem_post(semMemLobby);
	printf("gave semaphore back\n");

	int descripteur_pipe_lecture = 0;

	fprintf(stdout,"PID du processus : %d\n",getpid());
	sigset_t set;
	struct sigaction usr1, usr2;
	usr1.sa_handler= &sigusr_handler;
	usr2.sa_handler= &sigusr_handler;
	sigemptyset(&set);
	sigaction(SIGUSR1,&usr1,NULL);
	sigaction(SIGUSR2,&usr2,NULL);
	sigaddset(&set, SIGUSR1);
	int signal;
/*	printf("waiting for signal SIGUSR1\n");*/
	sigwait(&set,&signal);
/*	printf("sigwait ended\n");*/
	if(signal){

	
int pid = getpid();

char path_pipe_client[28];
snprintf(path_pipe_client, sizeof(PATH_PIPE)+sizeof(pid), "%s%i", PATH_PIPE, pid);
printf("ppc : %s\n", path_pipe_client);

		CHECK(
            descripteur_pipe_lecture = open(path_pipe_client,O_RDONLY | O_NONBLOCK),
            "Impossible d'ouvrir le tube nommé.\n"
    	);
		char message[CHAR_BUFFER_LENGTH] = ""; 
	
		read(descripteur_pipe_lecture,message,CHAR_BUFFER_LENGTH);
		fprintf(stdout,"Client - message reçu : '%s'\n",message);
   char * token = strtok(message, " ");
   // loop through the string to extract all other tokens
  int i=0;
   while( token != NULL ) {
      cards[i]=atoi(token);
      i++;
      token = strtok(NULL, " ");
   }
   nbCards = i;
   printf("waiting to start\n");
   sigemptyset( &set );
   sigaddset(&set, SIGUSR1);
   sigaddset(&set, SIGUSR2);
   playing = 1 ;
   int nbTours = 0;
   int descripteur_pipe_ecriture = 0;
   while(playing){
   sigwait(&set,&signal);
	sem_t *semMemCardsPile;
	
    if(nbTours == 0){
   	initMemoirePileCartes();
	if((semMemCardsPile= sem_open("/memCardsPile", O_CREAT, S_IRWXU, 1)) == SEM_FAILED)
	{	
		perror("can not open semMemCardsPile");
		exit(-1);
	}
	cardsPile = shmat(shmIdCardsPile, NULL, 0);
	}
	printf("shmIDcardsPile %d\n", shmIdCardsPile);
	printf("contenu cardsPile %s\n", cardsPile);
    system("clear");
     printf("==== %s ====\n", myName);
    switch(signal){
   case SIGUSR1: 
    sem_wait(semMemCardsPile);
	 //printf("contenu cardsPile %s\n", cardsPile);
	 readCardPile();
	 sem_post(semMemCardsPile);
     printf("I must start playing!!!\n");
     afficher_cartes();
     int index;
     printf("please enter the index of the card you want to play :\n"); 
     scanf("%i",&index);
     sem_wait(semMemCardsPile);
	 //TO DO FONCTION DE JEU
	 int cardPlayed = 0;
	 
	 cardPlayed = basicPlay(index, cardsPile);
	  
	 sem_post(semMemCardsPile);
     
     printf("to server : kill(%i, SIGUSR1);\n", pidServer);
     kill(pidServer, SIGUSR1);
     break;

     case SIGUSR2: //other players need to check the shm to see what card has been played
	 sem_wait(semMemCardsPile);
	 //printf("contenu cardsPile %s\n", cardsPile);
	 readCardPile();
	 sem_post(semMemCardsPile);
     break;
	//HERE
	}
	nbTours++;
}
    close(descripteur_pipe_lecture); // todo : pas au meilleur endroit, on peut le faire avant?
    fprintf(stdout,"Client - fermeture du programme..\n");
    return EXIT_SUCCESS;
}
}
void initMemoireLobby(){
	key_t cleSegment;
	CHECK(cleSegment=ftok("/memLobby", 1), "fack, can't create key");
	CHECK(shmId=shmget(cleSegment, 200 * sizeof(char), IPC_CREAT | SHM_R | SHM_W), "fack, can't create shm");
	struct shmid_ds shmid_stats;
   	shmctl(shmId, IPC_STAT, &shmid_stats);
	printf("PID du créateur:%i\n", shmid_stats.shm_cpid); 
	pidServer = shmid_stats.shm_cpid;
	printf("shm ID : %i \n", shmId);
	
}

void initMemoirePileCartes(){
	key_t cleSegment;
	CHECK(cleSegment=ftok("/memCardsPile", 1), "fack, can't create key");
	CHECK(shmIdCardsPile=shmget(cleSegment, 200 * sizeof(char), IPC_CREAT | SHM_R | SHM_W), "fack, can't create shm");
}

void sigusr_handler(int sig, siginfo_t *si, void* arg)
{
//rien à mettre dedans???
}


void afficher_cartes(){
int i;
for (i=0; i <nbCards; i++){
	printf("[%02i] %s\n", i, get_card_name(cards[i]));
}
}

void removeCard(int index){
for (;index < nbCards-1; index++){
	cards[index] = cards[index+1];
}
}
void input (char * string, int length){
	fgets(string, MAX+1, stdin);
	while (*string != '\n')
		string++;
	*string = '\0'; 
}

int basicPlay(int index, char * message){
	printf("basicPlay- index : %d\n", index);
	// todo : boucler si la valeur est inférieure
	if(cardsPile == "" || get_card_points(cards[index]) >= get_card_points(cartePrecedente)){
		sprintf(message, "%i ",cards[index]);
     	printf("you selected : %s\n", get_card_name(cards[index])); 
	 	fprintf(stdout,"Client - message envoyé au serveur : '%s'\n",message);
		strcat(cardsPile, message);
		removeCard(index);
		nbCards--;
	}else{
		printf("Enter the index of a higher value card\n");
	}
	return cards[index];
}

void readCardPile(){
	char * cardsPile2; // do we really need this, since strtok still deletes the shm?
	cardsPile2 = malloc(200); 
	strcpy(cardsPile2,cardsPile);
   	char * token = strtok(cardsPile2, " ");
   	int i = 0;
	if(token != NULL){
	if(cartePrecedente != -1){
		printf("%s \n", get_card_name(atoi(token)));
	}
	cartePrecedente = atoi(token);
	}
   	// loop through the string to extract all other tokens
/*   	while( token != NULL ) {*/
/*      printf("%s \n", get_card_name(atoi(token)));*/
/*	  token = strtok(NULL, " "); */
/*   	}*/
	
}

