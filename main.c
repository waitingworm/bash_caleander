#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void show_calendar(int year, int month) {
    char command[100];
    sprintf(command, "./calendar_system %d %d", year, month);
    system(command);
}

int main() {
    int choice;
    int year = 2024;
    int month = 3;
    int chat_choice;

    while (1) {
        printf("\n=== 메인 메뉴 ===\n");
        printf("1. 캘린더 보기\n");
        printf("2. 일정 추가\n");
        printf("3. 일정 보기\n");
        printf("4. 일정 삭제\n");
        printf("5. 이전 달\n");
        printf("6. 다음 달\n");
        printf("7. 특정 달로 이동\n");
        printf("8. 채팅\n");
        printf("9. 뽀모도로 타이머\n");
        printf("0. 종료\n");
        printf("선택: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                show_calendar(year, month);
                break;
            case 2:
                system("./calendar_schedule add");
                break;
            case 3:
                system("./calendar_schedule show");
                break;
            case 4:
                system("./calendar_schedule delete");
                break;
            case 5:
                month--;
                if (month < 1) {
                    month = 12;
                    year--;
                }
                show_calendar(year, month);
                break;
            case 6:
                month++;
                if (month > 12) {
                    month = 1;
                    year++;
                }
                show_calendar(year, month);
                break;
            case 7:
                printf("년도: ");
                scanf("%d", &year);
                printf("월: ");
                scanf("%d", &month);
                getchar();
                show_calendar(year, month);
                break;
            case 8:
                printf("\n=== 채팅 종류 선택 ===\n");
                printf("1. 기본 터미널 채팅\n");
                printf("2. 고급 채팅\n");
                printf("3. FTP 채팅\n");
                printf("선택: ");
                scanf("%d", &chat_choice);
                getchar(); // 버퍼 비우기

                switch(chat_choice) {
                    case 1:
                        system("./terminal_chat");
                        break;
                    case 2:
                        system("./chatclient");
                        break;
                    case 3:
                        system("./ftpclient");
                        break;
                    default:
                        printf("잘못된 선택입니다.\n");
                }
                break;
            case 9:
                system("./pomodoro_timer");
                break;
            case 0:
                printf("프로그램을 종료합니다.\n");
                return 0;
            default:
                printf("잘못된 선택입니다.\n");
        }
    }
    return 0;
} 