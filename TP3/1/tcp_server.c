/*******************************************************************************
 * SERVIDOR no porto 9000, à escuta de novos clientes.  Quando surgem
 * novos clientes os dados por eles enviados são lidos e descarregados no ecran.
 *******************************************************************************/
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

#define SERVER_PORT 9008
#define BUF_SIZE 1024


void process_client(int client_fd, int counter, struct sockaddr_in client_info);
void erro(char *msg);
void signalHandler(int sig);
void initilization();
void checkClient(int nread, int num);
void writeToClient();
void sum(int numbers[]);

int	client_addr_size, fd, client, aux = 0, client_pid;
bool nums_received = false;
char buffer[BUF_SIZE], message[100];
int nread = 0, aux, i, numbers[10];
float med;
struct sockaddr_in addr, client_addr;

int main() {
	initilization();

	while (1) {
	    // clean finished child processes, avoiding zombies
	    // must use WNOHANG or would block whenever a child process was working
	    while (waitpid(-1, NULL, WNOHANG) > 0);
	    // wait for new connection
	    client = accept(fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size);
	    if (client > 0) {
		    aux++;
    		if (fork() == 0) {
    		    close(fd);
    		    process_client(client, aux, client_addr);
    		    exit(0);
    		}
      		close(client);
    	}
	}
	return 0;
}

void process_client(int client_fd, int counter, struct sockaddr_in client_info) {

	printf("Client %d, connected from %s:%d\n", counter, inet_ntoa(client_info.sin_addr), client_info.sin_port);

	nread = read(client_fd, buffer, BUF_SIZE - 1);
	client_pid = (int) strtol(buffer, (char **)NULL, 10);
	printf("Client PID is %d\n", client_pid);

	while (1) {
		nread = read(client_fd, buffer, BUF_SIZE - 1);
	    checkClient(nread, counter);
	    buffer[nread] = '\0';
	    printf("Client %d, connected from %s:%d says: %s\n", counter, inet_ntoa(client_info.sin_addr), client_info.sin_port, buffer);

	    if (strcmp(buffer, "DADOS") == 0) {
	    	printf("Reciving numbers from client %d\n", counter);
	    	for (i = 0; i < 10; i++) {
	    	    nread = read(client_fd, buffer, BUF_SIZE - 1);
	    	    checkClient(nread, counter);
	    	    numbers[i] = (int)strtol(buffer, (char **)NULL, 10);
	    	    //printf("%d ", numbers[i]);
	    	    fflush(stdout);
	    	}
	    	nums_received = true;
	    	printf("\nNumbers received successfully from client %d\n", counter);
	    	
	    } else if (strcmp(buffer, "SOMA") == 0) {
	    	if(nums_received) {
	    		sum(numbers);
	    		writeToClient(buffer);
	    	} else
	    		writeToClient(buffer);
	    
	    } else if (strcmp(buffer, "MEDIA") == 0) {
	    	if(nums_received) {
	    		sum(numbers);
	      		med = aux / 10;
	      		writeToClient(buffer);
	      	} else
	      		writeToClient(buffer);
	    
	    } else if (strcmp(buffer, "SAIR") == 0) {
	    	printf("Client %d left\n", counter);
	    	break;
	    
	    } else
	    	printf("Erro: mensagem nao conhecida\n");

	    printf("\n");
	}
	close(client_fd);
}

void erro(char *msg) {
  printf("Erro: %s\n", msg);
  exit(-1);
}

void signalHandler(int sig) {
	printf("\nClosing sockets...\n");
	kill(client_pid, SIGTERM);
	close(client);
	char aux = 1;
	setsockopt(client, SOL_SOCKET, SO_REUSEADDR,&aux, sizeof(aux));
	exit(0);
}

void checkClient(int nread, int num) {
	if(nread == 0) {
		printf("\nClient %d disconnected\n", num);
		close(client);
		exit(0);
	}
}

void initilization() {

	signal(SIGINT, signalHandler);
	bzero((void *)&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(SERVER_PORT);

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	  	erro("na funcao socket");

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		erro("na funcao bind");

	if (listen(fd, 5) < 0) 
	  	erro("na funcao listen");

	client_addr_size = sizeof(client_addr);
	printf("Server open on %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
}

void sum(int numbers[]) {
	aux = 0;
	for (i = 0; i < 10; i++) {
		aux = aux + numbers[i];
	}
}

void writeToClient() {
	if (nums_received) {
		if (strcmp(buffer, "SOMA") == 0)
			snprintf(message, 100, "Soma=%d\n", aux);
		else if (strcmp(buffer, "MEDIA") == 0)
			snprintf(message, 100, "Media=%0.2f\n", med);
	} else
		snprintf(message, 100, "No numbers yet!\n");
	write(client, message, 1 + strlen(message));
}
