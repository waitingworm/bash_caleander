#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <fcntl.h>

#define DEFAULT_WORK_TIME 25 * 60  // 25분
#define DEFAULT_SHORT_BREAK 5 * 60  // 5분
#define DEFAULT_LONG_BREAK 15 * 60  // 15분
#define SESSIONS_BEFORE_LONG_BREAK 4

// 전역 변수
int work_time = DEFAULT_WORK_TIME;
int short_break = DEFAULT_SHORT_BREAK;
int long_break = DEFAULT_LONG_BREAK;
int remaining_time = DEFAULT_WORK_TIME;
int is_paused = 0;
int current_session = 1;
int total_sessions = 0;
int total_work_time = 0;
char current_task[256] = "작업 없음";

// 함수 선언
int kbhit(void);
void show_settings(void);
void show_statistics(void);
void start_timer(void);

// 터미널 설정 함수
void set_terminal_mode() {
    struct termios term;
    tcgetattr(0, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &term);
}

// 터미널 복원 함수
void reset_terminal_mode() {
    struct termios term;
    tcgetattr(0, &term);
    term.c_lflag |= ICANON | ECHO;
    tcsetattr(0, TCSANOW, &term);
}

// 타이머 표시 함수
void display_timer() {
    int minutes = remaining_time / 60;
    int seconds = remaining_time % 60;
    int total_time = (current_session % SESSIONS_BEFORE_LONG_BREAK == 0) ? long_break : 
                    (is_paused ? remaining_time : work_time);
    int progress = ((total_time - remaining_time) * 50) / total_time;
    
    printf("\033[2J\033[H");  // 화면 클리어
    printf("=== 뽀모도로 타이머 ===\n\n");
    printf("현재 작업: %s\n", current_task);
    printf("현재 세션: %d/%d\n", current_session, SESSIONS_BEFORE_LONG_BREAK);
    printf("총 작업 시간: %d분\n\n", total_work_time / 60);
    printf("남은 시간: %02d:%02d\n\n", minutes, seconds);
    
    // 진행률 바 표시
    printf("[");
    for (int i = 0; i < 50; i++) {
        if (i < progress) {
            printf("=");
        } else {
            printf(" ");
        }
    }
    printf("] %d%%\n\n", (progress * 100) / 50);
    
    printf("p: 일시정지/재개\n");
    printf("q: 종료\n");
    fflush(stdout);
}

// 세션 기록 저장 함수
void save_session_record() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char filename[256];
    sprintf(filename, "pomodoro_log_%04d%02d%02d.txt", 
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
    
    FILE *fp = fopen(filename, "a");
    if (fp) {
        fprintf(fp, "[%02d:%02d] 세션 %d 완료 - %s (작업 시간: %d분)\n",
                t->tm_hour, t->tm_min, current_session, current_task, work_time / 60);
        fclose(fp);
    }
}

// 통계 표시 함수
void show_statistics() {
    printf("\033[2J\033[H");  // 화면 클리어
    printf("=== 뽀모도로 통계 ===\n\n");
    
    // 오늘의 통계
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char filename[256];
    sprintf(filename, "pomodoro_log_%04d%02d%02d.txt", 
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
    
    FILE *fp = fopen(filename, "r");
    if (fp) {
        char line[512];
        printf("오늘의 작업 기록:\n");
        printf("----------------\n");
        while (fgets(line, sizeof(line), fp)) {
            printf("%s", line);
        }
        fclose(fp);
    }
    
    printf("\n총 작업 시간: %d분\n", total_work_time / 60);
    printf("완료한 세션 수: %d\n", total_sessions);
    printf("\nEnter를 눌러 계속...");
    getchar();
}

// 설정 표시 함수
void show_settings() {
    printf("\033[2J\033[H");  // 화면 클리어
    printf("=== 뽀모도로 설정 ===\n\n");
    
    printf("1. 작업 이름 설정\n");
    printf("2. 작업 시간 설정 (현재: %d분)\n", work_time / 60);
    printf("3. 짧은 휴식 시간 설정 (현재: %d분)\n", short_break / 60);
    printf("4. 긴 휴식 시간 설정 (현재: %d분)\n", long_break / 60);
    printf("5. 기본값으로 복원\n");
    printf("0. 돌아가기\n\n");
    
    printf("선택: ");
    int choice;
    scanf("%d", &choice);
    getchar();  // 개행 문자 제거
    
    switch (choice) {
        case 1:
            printf("\n작업 이름: ");
            fgets(current_task, sizeof(current_task), stdin);
            current_task[strcspn(current_task, "\n")] = 0;
            break;
        case 2:
            printf("\n작업 시간(분): ");
            int minutes;
            scanf("%d", &minutes);
            work_time = minutes * 60;
            remaining_time = work_time;
            break;
        case 3:
            printf("\n짧은 휴식 시간(분): ");
            scanf("%d", &minutes);
            short_break = minutes * 60;
            break;
        case 4:
            printf("\n긴 휴식 시간(분): ");
            scanf("%d", &minutes);
            long_break = minutes * 60;
            break;
        case 5:
            work_time = DEFAULT_WORK_TIME;
            short_break = DEFAULT_SHORT_BREAK;
            long_break = DEFAULT_LONG_BREAK;
            remaining_time = work_time;
            break;
    }
}

// 타이머 실행 함수
void start_timer() {
    set_terminal_mode();
    
    while (1) {
        display_timer();
        
        // 키 입력 확인
        if (kbhit()) {
            char key = getchar();
            if (key == 'p') {
                is_paused = !is_paused;
            } else if (key == 'q' && is_paused) {
                break;
            }
        }
        
        if (!is_paused) {
            if (remaining_time > 0) {
                sleep(1);
                remaining_time--;
            } else {
                // 세션 완료
                total_sessions++;
                total_work_time += work_time;
                save_session_record();
                
                if (current_session % SESSIONS_BEFORE_LONG_BREAK == 0) {
                    remaining_time = long_break;
                    printf("\n긴 휴식 시간입니다! %d분 휴식을 시작합니다.\n", long_break / 60);
                } else {
                    remaining_time = short_break;
                    printf("\n짧은 휴식 시간입니다! %d분 휴식을 시작합니다.\n", short_break / 60);
                }
                
                current_session = (current_session % SESSIONS_BEFORE_LONG_BREAK) + 1;
                sleep(3);
            }
        }
    }
    
    reset_terminal_mode();
}

int main() {
    while (1) {
        printf("\033[2J\033[H");  // 화면 클리어
        printf("=== 뽀모도로 타이머 ===\n\n");
        printf("1. 타이머 시작\n");
        printf("2. 설정\n");
        printf("3. 통계\n");
        printf("0. 종료\n\n");
        
        printf("선택: ");
        int choice;
        scanf("%d", &choice);
        getchar();  // 개행 문자 제거
        
        switch (choice) {
            case 1:
                start_timer();
                break;
            case 2:
                show_settings();
                break;
            case 3:
                show_statistics();
                break;
            case 0:
                return 0;
        }
    }
    
    return 0;
}

// kbhit 함수 구현
int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;
    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    
    ch = getchar();
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    
    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    
    return 0;
} 