#include "client.h"


int cards[DECK_SIZE/2]; // todo : set la taille de cards un peu mieux
int nbCards;
char *myName;
char * cardsPile;
int playing ;
int shmId; // shmId memLobby
int shmIdCardsPile;
int pidServer;
int cartePrecedente = -1;
int maDerniereCarte;
sem_t *semMemCardsPile;

int main(int argc,char * argv[]) 
{	
	signal(SIGINT,sigint_handler);
	printf("my pid: %i\n",getpid());
	initMemoireLobby();
	//inscription joueur dans le lobby 
	inscriptionJoueur();
	int descripteur_pipe_lecture = 0;

	fprintf(stdout,"PID du processus : %d\n",getpid());
	signal(SIGUSR1,sig_handler_empty);
	signal(SIGUSR2,sig_handler_empty);
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	int signal;
	printf("waiting for signal SIGUSR1\n");
	sigwait(&set,&signal);
	printf("sigwait ended\n");

	//traitement signal reçu
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
		//debut partie
   		while(playing){
   			sigwait(&set,&signal);
	
    		if(nbTours == 0){
   				initMemoirePileCartes();
				if((semMemCardsPile= sem_open("/memCardsPile", O_CREAT, S_IRWXU, 1)) == SEM_FAILED)
				{	
					perror("can not open semMemCardsPile");
					exit(-1);
				}
				cardsPile = shmat(shmIdCardsPile, NULL, 0);
			}
    		system("clear");
     		printf("==== %s ====\n", myName);
   			printf("cardsPile : [%s]\n", cardsPile); 

    		switch(signal){
   				case SIGUSR1:
     				sem_wait(semMemCardsPile);
	 				readCardPile();
	 				if(maDerniereCarte == cartePrecedente){
						printf("Plus personne ne peut jouer au dessus\n\n");
						cartePrecedente = -1;
						/*		strcpy(cardsPile,"");*/
						cardsPile[0] = '\0';
	 				}
	 				sem_post(semMemCardsPile);
     				printf("C'est mon tour !!!\n");
     				afficher_cartes();
     				int index;
	 
	 				do{
		 				printf("please enter the index of the card you want to play :\n"); 
     	 				scanf("%i",&index);
	 				}while(!(basicPlay(index)));
	 				if (nbCards == 0){
	      				printf("to server : kill(%i, SIGUSR2);\n", pidServer);
	     				kill(pidServer, SIGUSR2);
	     				exit(0);
	 				} else{
     					printf("to server : kill(%i, SIGUSR1);\n", pidServer);
     					kill(pidServer, SIGUSR1);
    				}
     				break;

     				case SIGUSR2: //other players need to check the shm to see what card has been played
	 					sem_wait(semMemCardsPile);
	 					if (index == 100){
		 					printf("Vous avez passé votre tour.\n");
	 					}
						readCardPile();
	
	 
	 					sem_post(semMemCardsPile);
     					break;
						//HERE
			}
		nbTours++;
		}
    close(descripteur_pipe_lecture); 
    fprintf(stdout,"Client - fermeture du programme..\n");
    return EXIT_SUCCESS;
	}
}
/**
 * Fonction : initMemoireLobby
 * Description : initalize la mémoire  partagée memLobby qui contient la liste des joueurs.
 */
void initMemoireLobby(){
	key_t cleSegment;
	CHECK(cleSegment=ftok("/memLobby", 1), "error, can't create key");
	CHECK(shmId=shmget(cleSegment, 200 * sizeof(char), IPC_CREAT | SHM_R | SHM_W), "fack, can't create shm");
	struct shmid_ds shmid_stats;
   	shmctl(shmId, IPC_STAT, &shmid_stats);
	printf("PID du créateur:%i\n", shmid_stats.shm_cpid); 
	pidServer = shmid_stats.shm_cpid;
	printf("shm ID : %i \n", shmId);
	
}
/**
 * Fonction : inscriptionJoueur
 * Description : permet d'inscrire un joueur dans la mémoire partagée memLobby
 */
void inscriptionJoueur(){
	sem_t *semMemLobby;
	if((semMemLobby= sem_open("/memLobby", O_CREAT, S_IRWXU, 1)) == SEM_FAILED)
	{	
		perror("can not open semMemLobby");
		exit(-1);
	}
	
	myName = (char *) malloc(MAX * sizeof(char));
	do{
		printf("Enter your name: ");
		input(myName, 10);
	} while (myName[0] == '\0');

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

}
/**
 * Fonction : initMemoirePileCartes
 * Description : initialize la mémoire partagée de la pile de cartes.
 */
void initMemoirePileCartes(){
	key_t cleSegment;
	CHECK(cleSegment=ftok("/memCardsPile", 1), "error, can't create key");
	CHECK(shmIdCardsPile=shmget(cleSegment, 200 * sizeof(char), IPC_CREAT | SHM_R | SHM_W), "error, can't create shm");
}

/**
 * Fonction : afficher_cartes
 * Description : permet d'afficher les cartes de la main d'un joueur
 */
void afficher_cartes(){
	int i;
	for (i=0; i <nbCards; i++){
		printf("[%02i] %s\n", i, get_card_name(cards[i]));
	}
}
/**
 * Fonction : removeCard
 * Description : permet de retirer une carte de la main du joueur 
 * Argument: index de la carte à retirer
 */
void removeCard(int index){
	for (;index < nbCards-1; index++){
		cards[index] = cards[index+1];
	}
}
/**
 * Fonction : input
 * Description : permet de récuperer le nom choisi à l'inscription du joueur;
 */
void input (char * string, int length){
	fgets(string, MAX+1, stdin);
	while (*string != '\n')
		string++;
	*string = '\0';  
}
/**
 * Fonction : basicPlay
 * Description : Check si la valeur de la carte choisie est supérieure à la carte précédente et 
 * l’ajoute à la pile si oui.
 * Arguments : index (index de la carte choisie)
 * Return : booléen
 */
bool basicPlay(int index){
	if(index != 100){
		printf("basicPlay- index : %d\n", index);
		maDerniereCarte = cards[index];
		if(cardsPile[0] == '\0' || get_card_points(cards[index]) > get_card_points(cartePrecedente)){
			char message[3];
			sprintf(message, "%i ",cards[index]);
			printf("you selected : %s\n", get_card_name(cards[index])); 
			fprintf(stdout,"Client - message envoyé au serveur : '%s'\n",message);
			sem_wait(semMemCardsPile);
			strcpy(cardsPile, "\0");
			strcat(cardsPile, message);
			sem_post(semMemCardsPile);
			removeCard(index);
			nbCards--;
		}else{
			printf("Enter the index of a higher value card\n");
			return false;
		}
	}
	return true;
}
/**
 * Fonction : readCardPile
 * Description : permet de lire le contenu de la mémoire et d'afficher la valeur de la dernière 
 * carte.
 * Return : 
 */
void readCardPile(){
	char * cardsPile2; 
	cardsPile2 = malloc(200); 
	strcpy(cardsPile2,cardsPile);
   	char * token = strtok(cardsPile2, " ");
   	int i = 0;
	if(token != NULL){
		cartePrecedente = atoi(token);
		printf("%s \n", get_card_name(cartePrecedente));
	}
}

/**
 * Fonction : sig_handler_empty
 * Description : permet de dérouter le signal 
 * l’afficher.
 */
void sig_handler_empty(){}

/**
 * Fonction : sigint_handler
 * Description : permet de dérouter le signal de fin de partie et de quitter le programme
 * l’afficher.
 */
void sigint_handler(){
	printf("partie terminée\n");
	exit(0);
}
