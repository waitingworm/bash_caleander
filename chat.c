#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define MAX_LINE 256
#define COLOR_RESET "\033[0m"
#define BORDER_LINE "┌─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐"
#define BORDER_LINE2 "└─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘"
#define BORDER_LINE3 "├─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┤"

const char* colors[] = {
    "\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"
};

int new_message_flag = 0;
char chat_filename[64] = "";
char chat_roomname[64] = "";
int color_enabled = 1;
char user_list[100][32];
int user_count = 0;

const char* get_color(const char* nickname) {
    if (!color_enabled) return "";
    int hash = 0;
    for (int i = 0; nickname[i]; i++) hash += nickname[i];
    return colors[hash % 6];
}

void add_user_if_new(const char* nickname) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(user_list[i], nickname) == 0) return;
    }
    if (user_count < 100) {
        strcpy(user_list[user_count++], nickname);
    }
}

void print_user_list() {
    printf("\n%s\n", BORDER_LINE);
    printf("│                                                                       👥  현재 채팅방 참여자 목록                                                                     │\n");
    printf("%s\n", BORDER_LINE3);
    for (int i = 0; i < user_count; i++) {
        printf("│ - %s%s%s\n", get_color(user_list[i]), user_list[i], COLOR_RESET);
    }
    printf("%s\n", BORDER_LINE2);
}

void print_help() {
    printf("\n%s\n", BORDER_LINE);
    printf("│                                                                       📋  사용 가능한 명령어                                                                          │\n");
    printf("%s\n", BORDER_LINE3);
    printf("│ send   - 메시지 전송\n");
    printf("│ read   - 채팅 기록 보기\n");
    printf("│ upload - 파일 업로드\n");
    printf("│ search - 파일 검색\n");
    printf("│ tagsearch - 태그로 검색\n");
    printf("│ change - 채팅방 변경\n");
    printf("│ users  - 참여자 목록\n");
    printf("│ help   - 도움말\n");
    printf("│ exit   - 종료\n");
    printf("%s\n", BORDER_LINE2);
}

void print_ui_header(const char* nickname) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char timestr[64];
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", t);
    
    printf("\n%s\n", BORDER_LINE);
    printf("│                                                                       💬  터미널 채팅                                                                                │\n");
    printf("%s\n", BORDER_LINE3);
    printf("│ 사용자: %s%s%s\n", get_color(nickname), nickname, COLOR_RESET);
    printf("│ 현재 시간: %s\n", timestr);
    printf("│ 현재 채팅방: %s (%s)\n", chat_roomname, chat_filename);
    printf("%s\n", BORDER_LINE3);
}

void receive_message(const char* nickname, const char* message) {
    new_message_flag = 1;
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char timestr[64];
    strftime(timestr, sizeof(timestr), "%H:%M:%S", t);

    printf("%s\n", BORDER_LINE3);
    printf("│ [%s] %s%s%s: %s\n", timestr, get_color(nickname), nickname, COLOR_RESET, message);

    FILE* chat_log = fopen(chat_filename, "a");
    if (chat_log) {
        fprintf(chat_log, "[%s] %s: %s\n", timestr, nickname, message);
        fclose(chat_log);
    }
}

void read_messages() {
    new_message_flag = 0;
    printf("\n%s\n", BORDER_LINE);
    printf("│                                                                       📜  채팅 기록                                                                                   │\n");
    printf("%s\n", BORDER_LINE3);
    FILE* chat_log = fopen(chat_filename, "r");
    if (chat_log) {
        char line[MAX_LINE];
        while (fgets(line, sizeof(line), chat_log)) {
            printf("│ %s", line);
        }
        fclose(chat_log);
    } else {
        printf("│ (채팅 기록이 없습니다)\n");
    }
    printf("%s\n", BORDER_LINE2);
}

void check_new_message_alert() {
    if (new_message_flag) {
        printf("\n%s\n", BORDER_LINE3);
        printf("│ 🔔 [알림] 새로운 메시지가 도착했습니다!\n");
    }
}

void search_file(const char* keyword) {
    FILE* fp = fopen("files.txt", "r");
    if (!fp) { perror("파일 열기 실패"); return; }

    char line[MAX_LINE];
    printf("\n[검색 결과: '%s']\n", keyword);
    int found = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, keyword)) {
            printf("%s", line);
            found = 1;
        }
    }
    if (!found) printf("(일치하는 설명/태그/파일명이 없습니다)\n");
    fclose(fp);
}

void tag_search_file(const char* tag) {
    FILE* fp = fopen("files.txt", "r");
    if (!fp) { perror("파일 열기 실패"); return; }
    char line[MAX_LINE];
    printf("\n[태그 검색 결과: '%s']\n", tag);
    int found = 0;
    while (fgets(line, sizeof(line), fp)) {
        char* tag_start = strchr(line, '[');
        char* tag_end = strchr(line, ']');
        if (tag_start && tag_end) {
            char tag_section[128];
            strncpy(tag_section, tag_start + 1, tag_end - tag_start - 1);
            tag_section[tag_end - tag_start - 1] = '\0';
            if (strstr(tag_section, tag)) {
                printf("%s", line);
                found = 1;
            }
        }
    }
    if (!found) printf("(일치하는 태그가 없습니다)\n");
    fclose(fp);
}

void upload_file(const char* nickname) {
    char filename[64], description[128], tags[128];
    printf("\n[파일 업로드]\n파일명: "); scanf("%s", filename); getchar();
    printf("설명: "); fgets(description, sizeof(description), stdin); description[strcspn(description, "\n")] = 0;
    printf("태그 (쉼표로 구분): "); fgets(tags, sizeof(tags), stdin); tags[strcspn(tags, "\n")] = 0;
    FILE* file_meta = fopen("files.txt", "a");
    if (file_meta) {
        fprintf(file_meta, "%s - %s [%s] (%s)\n", filename, description, tags, nickname);
        fclose(file_meta);
        printf("업로드 완료\n");
    } else printf("업로드 실패\n");
}

void select_chat_room() {
    int room;
    char roomname[32];
    char input[32];

    while (1) {
        printf("\n%s\n", BORDER_LINE);
        printf("│                                                                       🏠  채팅방 선택                                                                              │\n");
        printf("%s\n", BORDER_LINE3);
        printf("│ 1) 전기공학\n");
        printf("│ 2) 전자공학\n");
        printf("│ 3) 시스템공학\n");
        printf("│ 4) 새 채팅방 만들기\n");
        printf("%s\n", BORDER_LINE3);
        printf("│ 선택: ");
        fgets(input, sizeof(input), stdin);
        if (sscanf(input, "%d", &room) != 1) {
            printf("│ 잘못된 입력입니다. 숫자를 입력하세요.\n");
            continue;
        }

        if (room == 1) {
            strcpy(chat_filename, "chat_elec.txt");
            strcpy(chat_roomname, "전기공학");
        } else if (room == 2) {
            strcpy(chat_filename, "chat_elct.txt");
            strcpy(chat_roomname, "전자공학");
        } else if (room == 3) {
            strcpy(chat_filename, "chat_sys.txt");
            strcpy(chat_roomname, "시스템공학");
        } else if (room == 4) {
            printf("│ 새 채팅방 이름 입력: ");
            scanf("%s", roomname); getchar();
            snprintf(chat_filename, sizeof(chat_filename), "chat_%s.txt", roomname);
            snprintf(chat_roomname, sizeof(chat_roomname), "%s", roomname);
            printf("│ '%s' 채팅방에 입장합니다.\n", roomname);
        } else {
            printf("│ 잘못된 선택입니다. 1~4 중에서 선택하세요.\n");
            continue;
        }
        printf("%s\n", BORDER_LINE2);
        break;
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1 && strcmp(argv[1], "--no-color") == 0)
        color_enabled = 0;

    print_help();
    char command[64], nickname[32], message[128], keyword[64];
    printf("\n%s\n", BORDER_LINE);
    printf("│                                                                       👋  환영합니다!                                                                                 │\n");
    printf("%s\n", BORDER_LINE3);
    printf("│ 닉네임 입력: "); scanf("%s", nickname); getchar();
    add_user_if_new(nickname);

    while (strlen(chat_roomname) == 0) {
        select_chat_room();
    }

    while (1) {
        printf("\n%s\n", BORDER_LINE);
        printf("│                                                                       💬  터미널 채팅                                                                                │\n");
        printf("%s\n", BORDER_LINE3);
        print_ui_header(nickname);
        check_new_message_alert();
        printf("│ 명령어 입력: ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "send") == 0) {
            printf("│ 메시지: ");
            fgets(message, sizeof(message), stdin);
            message[strcspn(message, "\n")] = 0;
            receive_message(nickname, message);
        } else if (strcmp(command, "read") == 0) {
            read_messages();
        } else if (strcmp(command, "upload") == 0) {
            upload_file(nickname);
        } else if (strcmp(command, "search") == 0) {
            printf("│ 검색어: ");
            fgets(keyword, sizeof(keyword), stdin);
            keyword[strcspn(keyword, "\n")] = 0;
            search_file(keyword);
        } else if (strcmp(command, "tagsearch") == 0) {
            printf("│ 태그: ");
            fgets(keyword, sizeof(keyword), stdin);
            keyword[strcspn(keyword, "\n")] = 0;
            tag_search_file(keyword);
        } else if (strcmp(command, "change") == 0) {
            select_chat_room();
        } else if (strcmp(command, "users") == 0) {
            print_user_list();
        } else if (strcmp(command, "help") == 0) {
            print_help();
        } else if (strcmp(command, "exit") == 0) {
            printf("\n%s\n", BORDER_LINE);
            printf("│                                                                       👋  채팅을 종료합니다                                                                       │\n");
            printf("%s\n", BORDER_LINE2);
            break;
        } else {
            printf("│ 알 수 없는 명령어입니다. 'help'를 입력하여 사용 가능한 명령어를 확인하세요.\n");
        }
    }

    return 0;
} 