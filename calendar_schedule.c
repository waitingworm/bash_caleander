#include <stdio.h>
#include <stdlib.h>

#define SCHEDULE_FILE "schedules.txt"
#define MAX_SCHEDULES 1000

// 일정 구조체
typedef struct {
    int year;
    int month; 
    int day;
    char title[50];
} Schedule;

/* ========== 기본 문자열 함수들 ========== */

// 문자열 길이 구하기
int my_strlen(const char *str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// 문자열 복사
void my_strcpy(char *dest, const char *src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

// 안전한 문자열 복사 (길이 제한)
void my_safe_strcpy(char *dest, const char *src, int max_len) {
    int i = 0;
    while (i < max_len - 1 && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

// 문자열 비교
int my_strcmp(const char *str1, const char *str2) {
    int i = 0;
    while (str1[i] != '\0' && str2[i] != '\0') {
        if (str1[i] != str2[i]) {
            return str1[i] - str2[i];
        }
        i++;
    }
    return str1[i] - str2[i];
}

/* ========== 정렬 함수 ========== */

// 일정 비교 함수
int compare_schedules(const void *a, const void *b) {
    Schedule *sched_a = (Schedule *)a;
    Schedule *sched_b = (Schedule *)b;
    
    if (sched_a->year != sched_b->year) {
        return sched_a->year - sched_b->year;
    }
    if (sched_a->month != sched_b->month) {
        return sched_a->month - sched_b->month;
    }
    return sched_a->day - sched_b->day;
}

/* ========== 일정 관리 함수들 ========== */

// 일정 추가
void add_schedule(int year, int month, int day, const char* title) {
    FILE *file = fopen(SCHEDULE_FILE, "a");
    if (!file) {
        printf("파일을 열 수 없습니다.\n");
        return;
    }
    
    fprintf(file, "%d,%d,%d,%s\n", year, month, day, title);
    fclose(file);
    
    printf("일정이 추가되었습니다: %d년 %d월 %d일 - %s\n", year, month, day, title);
}

// 월별 일정 보기
void show_month_schedules(int year, int month) {
    FILE *file = fopen(SCHEDULE_FILE, "r");
    if (!file) {
        printf("이번 달에는 일정이 없습니다.\n");
        return;
    }
    
    Schedule schedules[MAX_SCHEDULES];
    char line[200];
    int count = 0;
    
    // 해당 월의 모든 일정 읽기
    while (fgets(line, sizeof(line), file) && count < MAX_SCHEDULES) {
        int s_year, s_month, s_day;
        char title[50];
        
        if (sscanf(line, "%d,%d,%d,%49[^\r\n]", &s_year, &s_month, &s_day, title) == 4) {
            if (s_year == year && s_month == month) {
                schedules[count].year = s_year;
                schedules[count].month = s_month;
                schedules[count].day = s_day;
                my_safe_strcpy(schedules[count].title, title, 50);
                count++;
            }
        }
    }
    fclose(file);
    
    if (count == 0) {
        printf("이번 달에는 일정이 없습니다.\n");
        return;
    }
    
    // 날짜순으로 정렬
    qsort(schedules, count, sizeof(Schedule), compare_schedules);
    
    printf("\n=== %d년 %d월 일정 목록 ===\n", year, month);
    for (int i = 0; i < count; i++) {
        printf("%2d일: %s\n", schedules[i].day, schedules[i].title);
    }
}

// 특정 날짜 일정 보기
void show_day_schedules(int year, int month, int day) {
    FILE *file = fopen(SCHEDULE_FILE, "r");
    if (!file) {
        printf("이 날에는 일정이 없습니다.\n");
        return;
    }
    
    char line[200];
    char schedules[10][50];
    int count = 0;
    
    // 해당 날짜의 모든 일정 수집
    while (fgets(line, sizeof(line), file) && count < 10) {
        int s_year, s_month, s_day;
        char title[50];
        
        if (sscanf(line, "%d,%d,%d,%49[^\r\n]", &s_year, &s_month, &s_day, title) == 4) {
            if (s_year == year && s_month == month && s_day == day) {
                my_safe_strcpy(schedules[count], title, 50);
                count++;
            }
        }
    }
    fclose(file);
    
    if (count == 0) {
        printf("이 날에는 일정이 없습니다.\n");
        return;
    }
    
    // 제목순으로 정렬 (간단한 버블 정렬)
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (my_strcmp(schedules[i], schedules[j]) > 0) {
                char temp[50];
                my_strcpy(temp, schedules[i]);
                my_strcpy(schedules[i], schedules[j]);
                my_strcpy(schedules[j], temp);
            }
        }
    }
    
    printf("\n=== %d년 %d월 %d일 일정 ===\n", year, month, day);
    for (int i = 0; i < count; i++) {
        printf("- %s\n", schedules[i]);
    }
}

// 날짜별 모든 일정 삭제
void delete_all_schedules_on_date(int year, int month, int day) {
    FILE *file = fopen(SCHEDULE_FILE, "r");
    if (!file) {
        printf("%d년 %d월 %d일에는 일정이 없습니다.\n", year, month, day);
        return;
    }
    
    char line[200];
    char schedules[10][50];
    int count = 0;
    
    // 삭제될 일정들을 먼저 수집
    while (fgets(line, sizeof(line), file) && count < 10) {
        int s_year, s_month, s_day;
        char title[50];
        
        if (sscanf(line, "%d,%d,%d,%49[^\r\n]", &s_year, &s_month, &s_day, title) == 4) {
            if (s_year == year && s_month == month && s_day == day) {
                my_safe_strcpy(schedules[count], title, 50);
                count++;
            }
        }
    }
    fclose(file);
    
    if (count == 0) {
        printf("%d년 %d월 %d일에는 일정이 없습니다.\n", year, month, day);
        return;
    }
    
    // 삭제될 일정 목록 보여주기
    printf("\n=== 삭제될 일정 목록 ===\n");
    for (int i = 0; i < count; i++) {
        printf("- %s\n", schedules[i]);
    }
    
    // 실제 삭제 수행
    file = fopen(SCHEDULE_FILE, "r");
    FILE *temp = fopen("temp.txt", "w");
    if (!file || !temp) {
        printf("파일 처리 중 오류가 발생했습니다.\n");
        return;
    }
    
    int deleted_count = 0;
    while (fgets(line, sizeof(line), file)) {
        int s_year, s_month, s_day;
        char s_title[50];
        
        if (sscanf(line, "%d,%d,%d,%49[^\r\n]", &s_year, &s_month, &s_day, s_title) == 4) {
            if (!(s_year == year && s_month == month && s_day == day)) {
                fprintf(temp, "%s", line);
            } else {
                deleted_count++;
            }
        } else {
            fprintf(temp, "%s", line);
        }
    }
    
    fclose(file);
    fclose(temp);
    
    remove(SCHEDULE_FILE);
    rename("temp.txt", SCHEDULE_FILE);
    
    printf("\n총 %d개의 일정이 삭제되었습니다.\n", deleted_count);
}

// 개별 일정 삭제
void delete_schedule(int year, int month, int day, const char* title) {
    FILE *file = fopen(SCHEDULE_FILE, "r");
    if (!file) {
        printf("일정 파일이 없습니다.\n");
        return;
    }
    
    FILE *temp = fopen("temp.txt", "w");
    if (!temp) {
        fclose(file);
        printf("임시 파일을 생성할 수 없습니다.\n");
        return;
    }
    
    char line[200];
    int deleted = 0;
    
    while (fgets(line, sizeof(line), file)) {
        int s_year, s_month, s_day;
        char s_title[50];
        
        if (sscanf(line, "%d,%d,%d,%49[^\r\n]", &s_year, &s_month, &s_day, s_title) == 4) {
            if (!(s_year == year && s_month == month && s_day == day && my_strcmp(s_title, title) == 0)) {
                fprintf(temp, "%s", line);
            } else {
                deleted = 1;
            }
        } else {
            fprintf(temp, "%s", line);
        }
    }
    
    fclose(file);
    fclose(temp);
    
    if (deleted) {
        remove(SCHEDULE_FILE);
        rename("temp.txt", SCHEDULE_FILE);
        printf("일정이 삭제되었습니다: %d년 %d월 %d일 - %s\n", year, month, day, title);
    } else {
        remove("temp.txt");
        printf("해당 일정을 찾을 수 없습니다.\n");
    }
}

/* ========== 메인 함수 ========== */

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("사용법: %s [명령] [인자들]\n", argv[0]);
        printf("명령:\n");
        printf("  add [년] [월] [일] [제목]  - 일정 추가\n");
        printf("  show [년] [월]            - 월별 일정 보기\n");
        printf("  day [년] [월] [일]        - 특정 날짜 일정 보기\n");
        printf("  delete [년] [월] [일]     - 날짜별 일정 삭제\n");
        printf("  remove [년] [월] [일] [제목] - 개별 일정 삭제\n");
        return 1;
    }
    
    if (my_strcmp(argv[1], "add") == 0 && argc >= 6) {
        int year = atoi(argv[2]);
        int month = atoi(argv[3]);
        int day = atoi(argv[4]);
        add_schedule(year, month, day, argv[5]);
    }
    else if (my_strcmp(argv[1], "show") == 0 && argc >= 4) {
        int year = atoi(argv[2]);
        int month = atoi(argv[3]);
        show_month_schedules(year, month);
    }
    else if (my_strcmp(argv[1], "day") == 0 && argc >= 5) {
        int year = atoi(argv[2]);
        int month = atoi(argv[3]);
        int day = atoi(argv[4]);
        show_day_schedules(year, month, day);
    }
    else if (my_strcmp(argv[1], "delete") == 0 && argc >= 5) {
        int year = atoi(argv[2]);
        int month = atoi(argv[3]);
        int day = atoi(argv[4]);
        delete_all_schedules_on_date(year, month, day);
    }
    else if (my_strcmp(argv[1], "remove") == 0 && argc >= 6) {
        int year = atoi(argv[2]);
        int month = atoi(argv[3]);
        int day = atoi(argv[4]);
        delete_schedule(year, month, day, argv[5]);
    }
    else {
        printf("잘못된 명령입니다.\n");
        return 1;
    }
    
    return 0;
}