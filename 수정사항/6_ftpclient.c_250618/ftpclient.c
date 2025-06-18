#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]) {
	int my_socket;
	struct sockaddr_in server_address;

	char ibuf[1024]= {0};
	FILE* file;

	if((my_socket= socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket 생성 실패\n");
		return -1;
	}

	server_address.sin_family= AF_INET;
	server_address.sin_port= htons(atoi(argv[2]));
	server_address.sin_addr.s_addr= inet_addr(argv[1]);
	if(inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
		perror("무효한 IP 주소\n");
		return -1;
	}

	if(connect(my_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
		perror("Server 연결 실패\n");
		return -1;
	}

	printf("Server 연결 성공\n");
	printf("Port: %hu\n", ntohs(server_address.sin_port));

	file= fopen("received", "wb");
	if(file == NULL) {
		perror("File 열람 실패\n");
		close(my_socket);
		return -1;
	}

	int B_receive;
	while((B_receive= recv(my_socket, ibuf, 1024, 0)) > 0) {
		fwrite(ibuf, 1, B_receive, file);
		memset(ibuf, 0, 1024);
	}
	
	printf("File 수신 완료\n", ibuf);

	fclose(file);
	close(my_socket);

	return 0;
}
