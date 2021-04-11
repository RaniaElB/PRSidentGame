#include "client.h"
#include <arpa/inet.h>
#include <netdb.h> /* getprotobyname */
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#define MAX 10
#define MESSAGE_SIZE 82
#define PATH_PIPE "./fifo-serveur-1"
#define CHAR_BUFFER_LENGTH 100

void initMemoireLobby();
void input();

char *myName;
int shmId;
int main(int argc,char * argv[]) 
{
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
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	int signal;
	sigwait(&set,&signal);
	
	if(signal){
		CHECK(
            descripteur_pipe_lecture = open(PATH_PIPE,O_RDONLY),
            "Impossible d'ouvrir le tube nommé.\n"
    	);

		char message[CHAR_BUFFER_LENGTH] = "";

		while(1){
			read(descripteur_pipe_lecture,message,CHAR_BUFFER_LENGTH);
			fprintf(stdout,"Client - message reçu : '%s'\n",message);
		}
	}

	
	sleep(15);
	
	
    close(descripteur_pipe_lecture);
    fprintf(stdout,"Client - fermeture du programme..\n");
    return EXIT_SUCCESS;

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

void input (char * string, int length){
	fgets(string, MAX+1, stdin);
	while (*string != '\n')
		string++;
	*string = '\0'; 
}

