#include "client.h"
#include <arpa/inet.h>
#include <netdb.h> /* getprotobyname */
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include "cards.h"

#define MAX 10
#define MESSAGE_SIZE 82
#define PATH_PIPE "./fifo-serveur-"
#define CHAR_BUFFER_LENGTH 100

void initMemoireLobby();
void input();
void usr1_handler();
int decode_msg_payload(char** raw_payload, int* decoded_payload, int max_elements); 
int cards[DECK_SIZE/2]; // todo : set la taille de cards un peu mieux
char *myName;
int shmId;
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
	struct sigaction usr1;
	usr1.sa_handler= &usr1_handler;
	sigemptyset(&set);
	sigaction(SIGUSR1,&usr1,NULL);
	sigaddset(&set, SIGUSR1);
	int signal;
/*	printf("waiting for signal SIGUSR1\n");*/
	sigwait(&set,&signal);
/*	printf("sigwait ended\n");*/
	if(signal){

	
int pid = getpid();

char path_pipe_client[28];
snprintf(path_pipe_client, sizeof(path_pipe_client)+sizeof(pid), "%s%i", PATH_PIPE, pid);
printf("ppc : %s\n", path_pipe_client);

		CHECK(
            descripteur_pipe_lecture = open(path_pipe_client,O_RDONLY),
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
   
	//HERE
	
	
    close(descripteur_pipe_lecture);
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
	printf("PID du créateur:%i\n", shmid_stats.shm_cpid);  // to save in order to send signal
	printf("shm ID : %i \n", shmId);
	
}
void usr1_handler(int sig, siginfo_t *si, void* arg)
{
//rien à mettre eud dans???
}

void input (char * string, int length){
	fgets(string, MAX+1, stdin);
	while (*string != '\n')
		string++;
	*string = '\0'; 
}

