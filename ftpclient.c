#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
	if (argc != 3) {
		printf("사용법: %s <서버IP> <포트번호>\n", argv[0]);
		return 1;
	}

	int my_socket;
	struct sockaddr_in server_address;
	char ibuf[1024] = {0};
	FILE* file;
	
	if ((my_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("Socket 생성 실패\n");
		exit(EXIT_FAILURE);
	}
	
	printf("Socket 생성 성공\n");
	
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(atoi(argv[2]));
	
	if (inet_pton(AF_INET, argv[1], &server_address.sin_addr) <= 0) {
		perror("IP 주소 변환 실패\n");
		close(my_socket);
		exit(EXIT_FAILURE);
	}
	
	if (connect(my_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
		perror("서버 연결 실패\n");
		close(my_socket);
		exit(EXIT_FAILURE);
	}
	
	printf("서버와 연결 성공\n");
	
	file = fopen("received_file", "wb");
	if (file == NULL) {
		perror("파일 생성 실패\n");
		close(my_socket);
		exit(EXIT_FAILURE);
	}
	
	int bytes_received;
	while ((bytes_received = recv(my_socket, ibuf, 1024, 0)) > 0) {
		if (fwrite(ibuf, 1, bytes_received, file) != (size_t)bytes_received) {
			printf("파일 쓰기 오류\n");
			break;
		}
		memset(ibuf, 0, 1024);
	}
	
	if (bytes_received < 0) {
		perror("데이터 수신 실패\n");
	} else {
		printf("파일 수신 완료\n");
	}
	
	fclose(file);
	close(my_socket);
	
	return 0;
}
