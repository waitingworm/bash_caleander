#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>

pthread_mutex_t mutex;
int clients_num;
int clients[100] = {[0 ... 99] = -1};
FILE* chat_log;
time_t tm;
struct tm current_time;

int space_available() {
	int i;
	for(i = 0; i < 100; i++) {
		if(clients[i] < 0)
			return i;
	}
	return -1;  // 공간이 없음
}

void pass_message(char* message, int message_length) {
	pthread_mutex_lock(&mutex);
	int i;
	char ct[64];  // 버퍼 크기 증가

	for(i = 0; i < 100; i++) {
		if(clients[i] >= 0 && message_length > 0) {
			if(write(clients[i], message, message_length) < 0) {
				perror("메시지 전송 실패");
				continue;
			}

			tm = time(NULL);
			current_time = *localtime(&tm);
			snprintf(ct, sizeof(ct), "%d-%02d-%02d %02d:%02d:%02d ", 
					current_time.tm_year + 1900, 
					current_time.tm_mon + 1, 
					current_time.tm_mday, 
					current_time.tm_hour, 
					current_time.tm_min, 
					current_time.tm_sec);
			fputs(ct, chat_log);
			fputs(message, chat_log);
			fflush(chat_log);
			
			printf("%d에게 다음 내용 전송: %s\n", clients[i], message);
		}
	}
	pthread_mutex_unlock(&mutex);
}

void* manage_client(void* client_socket) {
	int message_length;
	char message[1024];
	int sock = *(int*)client_socket;

	while((message_length = read(sock, message, sizeof(message) - 1)) > 0) {
		message[message_length] = '\0';
		if(message_length == 1 && message[0] == '\0') break;

		pass_message(message, message_length);
		printf("길이: %d, 연결 유지 중\n", message_length);
	}
	
	printf("%d의 연결 해제 발생\n", sock);
	
	pthread_mutex_lock(&mutex);
	int i;
	for(i = 0; i < 100; i++) {
		if(sock == clients[i]) {
			clients[i] = -1;
			break;
		}
	}
	--clients_num;
	printf("저런! 용사 1명이 낙오됐어요! 현재 %d명!\n", clients_num);
	pthread_mutex_unlock(&mutex);
	
	close(sock);
	free(client_socket);

	if(!clients_num) {
		fclose(chat_log);
	}
	return NULL;
}

int main(int argc, char* argv[]) {
	if(argc != 2) {
		printf("사용법: %s <포트번호>\n", argv[0]);
		return 1;
	}

	int server_socket, client_socket;
	struct sockaddr_in server_address, client_address;
	socklen_t client_address_size = sizeof(client_address);
	pthread_t clients_t[100];

	pthread_mutex_init(&mutex, NULL);
	server_socket = socket(PF_INET, SOCK_STREAM, 0);
	if(server_socket == -1) {
		perror("소켓 생성 실패");
		return 1;
	}

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(atoi(argv[1]));

	if(bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
		perror("바인드 실패");
		close(server_socket);
		return 1;
	}
	
	if(listen(server_socket, 10) == -1) {
		perror("리슨 실패");
		close(server_socket);
		return 1;
	}
	
	chat_log = fopen("chat_log.txt", "a");
	if(chat_log == NULL) {
		perror("로그 파일 열기 실패");
		close(server_socket);
		return 1;
	}

	printf("문을 열었어요! 오늘은 어떤 모험가가 사고를 칠까요? ></\n");
	
	while(1) {
		client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_size);
		if(client_socket == -1) {
			perror("클라이언트 연결 수락 실패");
			continue;
		}
		
		pthread_mutex_lock(&mutex);
		int available = space_available();
		if(available == -1) {
			printf("서버가 가득 찼습니다!\n");
			close(client_socket);
			pthread_mutex_unlock(&mutex);
			continue;
		}

		int* client_sock = malloc(sizeof(int));
		if(client_sock == NULL) {
			perror("메모리 할당 실패");
			close(client_socket);
			pthread_mutex_unlock(&mutex);
			continue;
		}
		*client_sock = client_socket;
		clients[available] = client_socket;
		
		printf("%d번 socket은 %d번에 할당했습니다.\n", client_socket, available);
		++clients_num;
		pthread_mutex_unlock(&mutex);
		
		pthread_create(&clients_t[available], NULL, manage_client, client_sock);
		pthread_detach(clients_t[available]);

		printf("모험가 등장! 파티원은 %d명이랍니다. IP: %s\n", clients_num, inet_ntoa(client_address.sin_addr));
	}

	close(server_socket);
	return 0;
}
