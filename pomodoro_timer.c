#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <fcntl.h>

#define WORK_TIME 25 * 60  // 25분
#define SHORT_BREAK 5 * 60  // 5분
#define LONG_BREAK 15 * 60  // 15분
#define SESSIONS_BEFORE_LONG_BREAK 4

// 전역 변수
int remaining_time = WORK_TIME;
int is_paused = 0;
int current_session = 1;
int total_sessions = 0;
int total_work_time = 0;

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
    printf("\033[2J\033[H");  // 화면 클리어
    printf("=== 뽀모도로 타이머 ===\n\n");
    printf("현재 세션: %d/%d\n", current_session, SESSIONS_BEFORE_LONG_BREAK);
    printf("총 작업 시간: %d분\n\n", total_work_time / 60);
    printf("남은 시간: %02d:%02d\n\n", minutes, seconds);
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
        fprintf(fp, "[%02d:%02d] 세션 %d 완료 (작업 시간: %d분)\n",
                t->tm_hour, t->tm_min, current_session, WORK_TIME / 60);
        fclose(fp);
    }
}

// 통계 표시 함수
void show_statistics() {
    printf("\033[2J\033[H");  // 화면 클리어
    printf("=== 뽀모도로 통계 ===\n\n");
    printf("오늘의 총 작업 시간: %d분\n", total_work_time / 60);
    printf("완료한 세션 수: %d\n", total_sessions);
    printf("\nEnter를 눌러 계속...");
    getchar();
}

int main() {
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
                total_work_time += WORK_TIME;
                save_session_record();
                
                if (current_session % SESSIONS_BEFORE_LONG_BREAK == 0) {
                    remaining_time = LONG_BREAK;
                    printf("\n긴 휴식 시간입니다! 15분 휴식을 시작합니다.\n");
                } else {
                    remaining_time = SHORT_BREAK;
                    printf("\n짧은 휴식 시간입니다! 5분 휴식을 시작합니다.\n");
                }
                
                current_session = (current_session % SESSIONS_BEFORE_LONG_BREAK) + 1;
                sleep(3);
            }
        }
    }
    
    reset_terminal_mode();
    show_statistics();
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