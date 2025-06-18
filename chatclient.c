#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define MAX_NAME_LEN 15
#define MAX_MSG_LEN 1000
#define MAX_IP_LEN 15
#define MAX_PORT_LEN 5

char my_name[MAX_NAME_LEN + 1];
char message[MAX_MSG_LEN + 1];
char server_port[MAX_PORT_LEN + 1];
char my_IP[MAX_IP_LEN + 1];

void* send_message(void* server_socket) {
	char my_information[128];
	char name_message[MAX_MSG_LEN + MAX_NAME_LEN + 5];
	int sock = *(int*)server_socket;
	
	snprintf(my_information, sizeof(my_information), "앗! 야생의 [%s]가 나타났다! IP: %s\n", my_name, my_IP);
	if(write(sock, my_information, strlen(my_information)) < 0) {
		perror("초기 메시지 전송 실패");
		return NULL;
	}

	while(1) {
		if(fgets(message, sizeof(message), stdin) == NULL) {
			break;
		}

		if(!strncmp(message, "::quit", 6)) {
			if(write(sock, "\0", 1) < 0) {
				perror("종료 메시지 전송 실패");
			}
			close(sock);
			exit(0);
		}

		snprintf(name_message, sizeof(name_message), "[%s]: %s", my_name, message);
		if(write(sock, name_message, strlen(name_message) + 1) < 0) {
			perror("메시지 전송 실패");
			break;
		}
	}
	return NULL;
}

void* receive_message(void* server_socket) { 
	char name_message[MAX_MSG_LEN + MAX_NAME_LEN + 5];
	int message_length;
	int sock = *(int*)server_socket;

	while(1) {
		message_length = read(sock, name_message, sizeof(name_message) - 1);
		if(message_length <= 0) {
			if(message_length < 0) {
				perror("메시지 수신 실패");
			}
			break;
		}

		name_message[message_length] = '\0';
		fputs(name_message, stdout);
		fflush(stdout);
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	if(argc != 4) {
		printf("사용법: %s <서버IP> <포트번호> <닉네임>\n", argv[0]);
		return 1;
	}

	int server_socket;
	struct sockaddr_in server_address;
	pthread_t send_t, receive_t;
	void* t_terminate;
	
	strncpy(my_IP, argv[1], MAX_IP_LEN);
	strncpy(server_port, argv[2], MAX_PORT_LEN);
	strncpy(my_name, argv[3], MAX_NAME_LEN);
	
	if((server_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("소켓 생성 실패");
		return 1;
	}
	
	printf("어디로든 문!\n");
	
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr(my_IP);
	server_address.sin_port = htons(atoi(server_port));

	if(connect(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
		perror("서버 연결 실패");
		close(server_socket);
		return 1;
	}

	int* sock_ptr = malloc(sizeof(int));
	if(sock_ptr == NULL) {
		perror("메모리 할당 실패");
		close(server_socket);
		return 1;
	}
	*sock_ptr = server_socket;

	pthread_create(&send_t, NULL, send_message, sock_ptr);
	pthread_create(&receive_t, NULL, receive_message, sock_ptr);
	
	pthread_join(send_t, &t_terminate);
	pthread_join(receive_t, &t_terminate);

	free(sock_ptr);
	close(server_socket);
	return 0;
}
