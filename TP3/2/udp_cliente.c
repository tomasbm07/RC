#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>

#define BUFLEN 512	// Tamanho do buffer
#define PORT 9000


struct hostent *hostPtr;	// Porto para recepção das mensagens


void erro(char *s) {
	perror(s);
	exit(1);
}


int main(int argc, char *argv[]) {
  struct sockaddr_in si_minha;

	int s, send_len, recv_len;
	socklen_t slen = sizeof(si_minha);
	char buf[BUFLEN];

    if ((hostPtr = gethostbyname(argv[1])) == 0)
		erro("Não consegui obter endereço");

	// Cria um socket para envio/recepção de pacotes UDP
	if((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		erro("Erro na criação do socket");
	}

  // Preenchimento da socket address structure
	si_minha.sin_family = AF_INET;
	si_minha.sin_port = htons(PORT);
	si_minha.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;

    strcpy(buf, argv[3]);

	if((send_len = sendto(s, buf, BUFLEN, 0, (struct sockaddr *) &si_minha, slen)) == -1) {
	  erro("Erro no sendto");
	}

    if((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_minha, (socklen_t *)&slen)) == -1) {
	  erro("Erro no recvfrom");
	}

    buf[recv_len] = '\0';

    printf("A mensagem enviada tem %s carateres\n", buf);

	// Fecha socket e termina programa
	close(s);
	return 0;
}