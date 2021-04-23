/**********************************************************************
 * CLIENTE liga ao servidor (definido em argv[1]) no porto especificado
 * (em argv[2]), escrevendo a palavra predefinida (em argv[3]).
 * USO: >cliente <enderecoServidor>  <porto>  <Palavra>
 **********************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>

#define BUF_SIZE 1024

void erro(char *msg);
void signalHandler(int sig);
void checkServerRead(int nread);
void onServerClose(int sig);
void readValueServer();
void nextInstruction();

int fd;
char buffer[BUF_SIZE], instruction[100];
int nread, teste = 0;

int main(int argc, char *argv[]) {
	char endServer[100];
	struct sockaddr_in addr;
	struct hostent *hostPtr;

	signal(SIGINT, signalHandler);
	signal(SIGPIPE, onServerClose);
	signal(SIGTERM, onServerClose);

	if (argc != 3) {
		printf("client <host> <port>\n");
		exit(-1);
	}

	strcpy(endServer, argv[1]);
	if ((hostPtr = gethostbyname(endServer)) == 0)
		erro("Não consegui obter endereço");

	bzero((void *) &addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
	addr.sin_port = htons((short) atoi(argv[2]));

	if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
		erro("socket");
	if(connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0)
		erro("Connect");

	//send server PID
	sprintf(instruction, "%d", getpid());
	write(fd, instruction, 1 + strlen(instruction));

	while(1) {
		//send messsage to server
		nextInstruction();
		write(fd, instruction, 1 + strlen(instruction));

		if(strcmp(instruction, "DADOS")==0) {
			for(int i=0; i<10; i++){
				fgets(buffer, BUF_SIZE, stdin);
				write(fd, buffer, 1+strlen(buffer));			}
		} else if(strcmp(instruction, "SOMA")==0 || strcmp(instruction, "MEDIA")==0) {
			readValueServer();
		} else if(strcmp(instruction, "SAIR")==0)
			break;
		else
			printf("Erro: comando desconhecido\n");
	}
	close(fd);
	exit(0);
}

void erro(char *msg) {
	printf("Erro: %s\n", msg);
	exit(-1);
}

void signalHandler(int sig) {
	printf("\n");
	close(fd);
	exit(0);
}

void checkServerRead(int nread){
	if(nread == 0){
		printf("Server is closed\n");
		close(fd);
		exit(0);
	}
}

void onServerClose(int sig){
	printf("Server is closed\n");
	//close(fd);
	exit(0);
}


void readValueServer() {
	nread = read(fd, buffer, BUF_SIZE-1);
	checkServerRead(nread);
	buffer[nread] = '\0';
	printf("%s\n", buffer);
}

void nextInstruction() {
	printf("Instruction: ");
	fgets(instruction, sizeof(instruction), stdin);
	instruction[strcspn(instruction, "\n")] = 0;
}


