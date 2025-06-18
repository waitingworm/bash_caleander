#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

char my_name[16];
char message[1008];
char server_port[8];
char my_IP[16];

void* send_message(void* server_socket) {
	char my_information[64];
	char name_message[1024];
	
	sprintf(my_information, "앗! 야생의 [%s]가 나타났다! IP: %s\n", my_name, my_IP);
	fflush(stdin);
	write(*(int*)server_socket, my_information, strlen(my_information));

	while(1) {
		printf("> "); // 입력 프롬프트
		fflush(stdout);
		if(fgets(message, 1008, stdin) == NULL) break;
		fflush(stdin);

		// 입력 줄 지우기 (커서를 위로 올리고 줄 삭제)
		printf("\033[A\33[2K\r");

		if(!strncmp(message, "::quit", 6)) {
			write(*(int*)server_socket, "\0", 1);
			close(*(int*)server_socket);
			exit(0);
		}

		sprintf(name_message, "[%s]: %s", my_name, message);
		write(*(int*)server_socket, name_message, strlen(name_message)+1);
	}
}

void* receive_message(void* server_socket) { 
	char name_message[1024];
	int message_length;

	while(1) {
		message_length= read(*(int*)server_socket, name_message, 1023);
		if(message_length == -1)
			return (void*)-1;

		name_message[message_length]= '\0';

		// 내가 보낸 메시지만 출력
		char my_prefix[32];
		sprintf(my_prefix, "[%s]:", my_name);
		if(strncmp(name_message, my_prefix, strlen(my_prefix)) == 0) {
			fputs(name_message, stdout);
		}
		// else: 아무것도 출력하지 않음 (주석 처리)
	}
}


int main(int argc, char *argv[]) {
	int server_socket;
	struct sockaddr_in server_address;
	pthread_t send_t, receive_t;
	void* t_terminate;
	
	sprintf(my_IP, "%s", argv[1]);
	sprintf(server_port, "%s", argv[2]);
	sprintf(my_name, "%s", argv[3]);
	if((server_socket= socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Failed to create socket.\n");
		exit(1);
	}
	
	printf("어디로든 문!\n");
	
	server_address.sin_family= AF_INET;
	server_address.sin_addr.s_addr= inet_addr(my_IP);
	server_address.sin_port= htons(atoi(server_port));

	if(connect(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
		printf("앗! 너무 늦었네요! 파티원들은 이미 모험을 떠났답니다! :/\n");
		exit(1);
	}

	pthread_create(&send_t, NULL, send_message, (void*)&server_socket);
	pthread_create(&receive_t, NULL, receive_message, (void*)&server_socket);
	pthread_join(send_t, &t_terminate);
	pthread_join(receive_t, &t_terminate);

	close(server_socket);
	return 0;
}
