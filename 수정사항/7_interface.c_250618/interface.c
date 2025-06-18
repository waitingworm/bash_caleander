#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	char port[8]= "8080";
	char ip[16]= "0.0.0.0";
	char command[100];
	int CHAT, FTP;
	int select;
	
	system("clear");
	printf("chat: 1, ftp: 2\n");

	switch(scanf("%d", &select)) {
		case 1: 
			strcpy(command, "./chat_server.exe ");
			strcat(command, port);
			system(command);
		case 2: 
			strcpy(command, "./ftp_server.exe ");
			strcat(command, port);
			system(command);
		default: return 0;
	}
	
	printf("exit\n");
	return 0;
}
