/* [message (1).txt 기반 코드 시작] */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define MAX_LINE 256
#define COLOR_RESET "\033[0m"
#define BORDER_LINE "==============================================="

// 사용자 닉네임별 색상
const char* colors[] = {
    "\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"
};
int new_message_flag = 0;
char chat_filename[64] = "chat_log.txt";
char chat_roomname[64] = "기본 채팅방";
int color_enabled = 1;

// 닉네임 해시 값에 따라 색상 반환
const char* get_color(const char* nickname) {
    if (!color_enabled) return "";
    int hash = 0;
    for (int i = 0; nickname[i]; i++) hash += nickname[i];
    return colors[hash % 6];
}

// 명령어 목록 출력
void print_help() {
    printf("\n== 사용 가능한 명령어 안내 ==\n");
    printf(" send     - 메시지 보내기\n");
    printf(" read     - 전체 대화 읽기\n");
    printf(" upload   - 파일 정보 업로드\n");
    printf(" search   - 파일 정보 검색 (키워드)\n");
    printf(" tagsearch- 파일 정보 검색 (태그)\n");
    printf(" tags     - 전체 태그 목록 보기\n");
    printf(" delete   - 내가 올린 파일 정보 삭제\n");
    printf(" change   - 채팅방 변경하기\n");
    printf(" help     - 도움말 보기\n");
    printf(" exit     - 채팅 종료\n");
}

// 상단 UI 헤더 출력
void print_ui_header(const char* nickname) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char timestr[64];
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", t);
    printf("\n%s\n사용자: %s%s%s\n현재 시간: %s\n현재 채팅방: %s (%s)\n%s\n",
           BORDER_LINE, get_color(nickname), nickname, COLOR_RESET, timestr, chat_roomname, chat_filename, BORDER_LINE);
}

// 메시지 수신 및 파일에 기록
void receive_message(const char* nickname, const char* message) {
    new_message_flag = 1;
    printf("%s%s%s: %s\n", get_color(nickname), nickname, COLOR_RESET, message);
    FILE* chat_log = fopen(chat_filename, "a");
    if (chat_log) {
        fprintf(chat_log, "%s: %s\n", nickname, message);
        fclose(chat_log);
    }
}

// 채팅 로그 읽기
void read_messages() {
    new_message_flag = 0;
    printf("\n[채팅 기록]\n");
    FILE* chat_log = fopen(chat_filename, "r");
    if (chat_log) {
        char line[MAX_LINE];
        while (fgets(line, sizeof(line), chat_log)) {
            printf("%s", line);
        }
        fclose(chat_log);
    } else {
        printf("(채팅 기록이 없습니다)\n");
    }
}

// 새 메시지 알림
void check_new_message_alert() {
    if (new_message_flag) {
        printf("\n[새 메시지 도착]\n");
    }
}

// 파일 정보 검색
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
    if (!found) printf("(일치하는 결과가 없습니다)\n");
    fclose(fp);
}

// 태그로 파일 정보 검색
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

// 파일 정보 업로드
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

// 파일 정보 삭제
void delete_file(const char* nickname) {
    char filename[64];
    printf("\n[파일 삭제]\n삭제할 파일명: "); scanf("%s", filename);
    FILE* fp = fopen("files.txt", "r");
    FILE* temp = fopen("temp.txt", "w");
    if (!fp || !temp) { perror("파일 열기 실패"); return; }
    char line[MAX_LINE]; int found = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, filename) && strstr(line, nickname)) { found = 1; continue; }
        fputs(line, temp);
    }
    fclose(fp); fclose(temp);
    remove("files.txt"); rename("temp.txt", "files.txt");
    if (found) printf("삭제 완료\n"); else printf("삭제 실패: 권한 또는 파일 없음\n");
}

// 모든 태그 목록 보기 및 선택 검색
void list_tags() {
    FILE* fp = fopen("files.txt", "r");
    if (!fp) { perror("파일 열기 실패"); return; }
    char line[MAX_LINE], all_tags[100][64]; int tag_count = 0;
    while (fgets(line, sizeof(line), fp)) {
        char* start = strchr(line, '[');
        char* end = strchr(line, ']');
        if (start && end) {
            char section[128];
            strncpy(section, start + 1, end - start - 1);
            section[end - start - 1] = '\0';
            char* token = strtok(section, ",");
            while (token) {
                while (*token == ' ') token++;
                int dup = 0;
                for (int i = 0; i < tag_count; i++) if (strcmp(all_tags[i], token) == 0) dup = 1;
                if (!dup && tag_count < 100) strcpy(all_tags[tag_count++], token);
                token = strtok(NULL, ",");
            }
        }
    }
    fclose(fp);
    if (tag_count == 0) { printf("(등록된 태그 없음)\n"); return; }
    printf("\n[태그 목록]\n");
    for (int i = 0; i < tag_count; i++) printf("%d) %s\n", i + 1, all_tags[i]);
    int choice;
    printf("\n검색할 태그 번호 선택 (0: 취소): "); scanf("%d", &choice); getchar();
    if (choice > 0 && choice <= tag_count) tag_search_file(all_tags[choice - 1]);
    else if (choice != 0) printf("잘못된 번호\n");
}

// 채팅방 선택 또는 생성
void select_chat_room() {
    int room;
    char roomname[32];
    char input[32];
    printf("\n[채팅방 선택]\n1) 전기공학\n2) 전자공학\n3) 시스템공학\n4) 새 채팅방 만들기\n선택: ");
    fgets(input, sizeof(input), stdin);
    if (sscanf(input, "%d", &room) != 1) {
        printf("입력이 잘못되었습니다. 기본 채팅방으로 연결합니다.\n");
        strcpy(chat_filename, "chat_log.txt");
        strcpy(chat_roomname, "기본 채팅방");
        return;
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
        printf("채팅방 이름 입력: ");
        scanf("%s", roomname);
        getchar();
        snprintf(chat_filename, sizeof(chat_filename), "chat_%s.txt", roomname);
        snprintf(chat_roomname, sizeof(chat_roomname), "%s", roomname);
        printf("'%s' 채팅방 입장\n", roomname);
    } else {
        printf("잘못된 선택 → 기본 채팅방 연결\n");
        strcpy(chat_filename, "chat_log.txt");
        strcpy(chat_roomname, "기본 채팅방");
    }
}

// 메인 함수
int main(int argc, char* argv[]) {
    // --no-color 옵션 처리
    if (argc > 1 && strcmp(argv[1], "--no-color") == 0)
        color_enabled = 0;
    
    char command[64], nickname[32], message[128], keyword[64];

    print_help();
    printf("\n환영합니다! 닉네임을 입력하세요: ");
    scanf("%s", nickname);
    getchar(); // 버퍼 비우기

    select_chat_room();

    while (1) {
        printf("\n(채팅방을 바꾸려면 'change'를 입력하세요)\n");
        print_ui_header(nickname);
        check_new_message_alert();

        printf("\n명령어를 입력하세요: ");
        scanf("%s", command);
        getchar(); // 버퍼 비우기

        if (strcmp(command, "send") == 0) {
            printf("메시지 입력: ");
            fgets(message, sizeof(message), stdin);
            message[strcspn(message, "\n")] = 0; // 개행 문자 제거
            receive_message(nickname, message);
        } else if (strcmp(command, "read") == 0) {
            read_messages();
        } else if (strcmp(command, "upload") == 0) {
            upload_file(nickname);
        } else if (strcmp(command, "search") == 0) {
            printf("검색어 입력: ");
            scanf("%s", keyword);
            search_file(keyword);
        } else if (strcmp(command, "tagsearch") == 0) {
            printf("태그 입력: ");
            scanf("%s", keyword);
            tag_search_file(keyword);
        } else if (strcmp(command, "tags") == 0) {
            list_tags();
        } else if (strcmp(command, "delete") == 0) {
            delete_file(nickname);
        } else if (strcmp(command, "change") == 0) {
            select_chat_room();
        } else if (strcmp(command, "help") == 0) {
            print_help();
        } else if (strcmp(command, "exit") == 0) {
            printf("채팅 프로그램을 종료합니다.\n");
            break;
        } else {
            printf("잘못된 명령어입니다. 'help'를 입력하면 명령어 목록을 볼 수 있습니다.\n");
        }
    }

    return 0;
}
/* [message (1).txt 기반 코드 종료] */