#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
	
	int my_socket, client_socket;
	struct sockaddr_in my_address, client_address;
	
	char ibuf[1024]= {0};
	FILE* file;
	
	my_address.sin_family= AF_INET;
	my_address.sin_addr.s_addr= INADDR_ANY;
	my_address.sin_port= htons(atoi(argv[2]));
	
	if((my_socket= socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("Socket 생성 실패\n");
		exit(EXIT_FAILURE);
	}

	printf("Socket 생성 성공\n");
	
	if(bind(my_socket, (struct sockaddr*)&my_address, sizeof(my_address)) < 0) {
		perror("Network 주소 할당 실패\n");
		close(my_socket);
		exit(EXIT_FAILURE);
	}
	
	printf("Server 주소 할당 성공\n");
	char tmp[32];
	inet_ntop(AF_INET, &my_address.sin_addr.s_addr, tmp, sizeof(tmp));
	printf("IP: %s\n", tmp);
	printf("Port: %hu\n", ntohs(my_address.sin_port));

	if(listen(my_socket, 3) < 0) {
		perror("Client의 요청 수행 불가\n");
		close(my_socket);
		exit(EXIT_FAILURE);
	}
	
	printf("Client의 요청 대기\n");
	int size_client_address= sizeof(client_address);
	client_socket= accept(my_socket, (struct sockaddr*)&client_address, (socklen_t*)&client_address);
	if(client_socket < 0) {
		perror("Client의 무효한 요쳥\n");
		close(my_socket);
		exit(EXIT_FAILURE);
	}
	
	printf("Client와 연결 성공\n");

	file= fopen(argv[1], "rb");
	if(file == NULL) {
		perror("File 열람 불가\n");
		close(client_socket);
		close(my_socket);
		exit(EXIT_FAILURE);
	}

	int B_read;
	while((B_read= fread(ibuf, 1, 1024, file)) > 0) {
		send(client_socket, ibuf, B_read, 0);
		memset(ibuf, 0, 1024);
	}

	printf("%s 전송 완료\n", argv[1]);

	fclose(file);
	close(client_socket);
	close(my_socket);

	return 0;
}
