#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <locale.h>

// 색상 코드 정의
#define COLOR_RED     "\033[31m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_RESET   "\033[0m"
#define COLOR_BG_WHITE "\033[47m\033[30m"

// 캘린더 레이아웃 설정
#define CELL_WIDTH 16
#define TOTAL_CELLS 7
#define BORDER_COUNT 8
#define CALENDAR_WIDTH (TOTAL_CELLS * CELL_WIDTH + BORDER_COUNT + 1)

// 일정 구조체
typedef struct {
    int year;
    int month;
    int day;
    char title[50];
} Schedule;

/* ========== 날짜 계산 함수들 ========== */

// 오늘 날짜 구하기
void get_today(int *year, int *month, int *day) {
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    *year = local->tm_year + 1900;
    *month = local->tm_mon + 1;
    *day = local->tm_mday;
}

// 윤년 판별
int is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// 월별 일수 계산
int days_in_month(int year, int month) {
    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && is_leap_year(year)) return 29;
    return days[month - 1];
}

// 요일 계산 (0: 일요일, 6: 토요일)
int get_day_of_week(int year, int month, int day) {
    struct tm time_struct = {0};
    time_struct.tm_year = year - 1900;
    time_struct.tm_mon = month - 1;
    time_struct.tm_mday = day;
    mktime(&time_struct);
    return time_struct.tm_wday;
}

/* ========== UTF-8 한글 처리 함수들 ========== */

// UTF-8 문자의 바이트 수 정확 계산
int utf8_char_bytes(const char *str) {
    if (!str || !(*str)) return 0;
    
    unsigned char c = (unsigned char)str[0];
    
    if (c < 0x80) {
        return 1;  // ASCII
    } else if ((c & 0xE0) == 0xC0) {
        return 2;  // 2바이트 UTF-8
    } else if ((c & 0xF0) == 0xE0) {
        return 3;  // 3바이트 UTF-8 (한글)
    } else if ((c & 0xF8) == 0xF0) {
        return 4;  // 4바이트 UTF-8
    } else {
        return 1;  // 잘못된 UTF-8, 1바이트로 처리
    }
}

// UTF-8 문자열의 실제 출력 너비 계산 (한글=2, 영문=1)
int get_string_width(const char *str) {
    if (!str) return 0;
    
    int width = 0;
    int i = 0;
    
    while (str[i]) {
        int bytes = utf8_char_bytes(&str[i]);
        
        if (bytes == 0) break;  // 잘못된 문자
        
        if (bytes == 3) {
            // 한글 (3바이트 UTF-8) - 2칸 차지
            width += 2;
        } else {
            // 영문/숫자/기호 (1바이트) - 1칸 차지
            width += 1;
        }
        
        i += bytes;
    }
    
    return width;
}

// 안전한 문자열 복사 함수 (strncpy 대체)
void safe_strcpy(char *dest, const char *src, int max_len) {
    int i;
    for (i = 0; i < max_len - 1 && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

// 안전한 UTF-8 문자열 자르기 (한글 깨짐 방지)
void truncate_to_width(const char *src, char *dest, int max_width) {
    int width = 0;
    int i = 0, j = 0;
    int last_safe_pos = 0;  // 마지막으로 안전한 위치
    int last_safe_width = 0;
    
    while (src[i] && width <= max_width) {
        int bytes = utf8_char_bytes(&src[i]);
        int char_width = (bytes == 3) ? 2 : 1;  // 한글=2, 영문=1
        
        // 다음 문자를 추가했을 때 최대 너비를 초과하는지 확인
        if (width + char_width <= max_width) {
            // 안전한 위치 기록
            last_safe_pos = j;
            last_safe_width = width;
            
            // 문자 복사
            for (int k = 0; k < bytes; k++) {
                dest[j++] = src[i++];
            }
            width += char_width;
        } else {
            break;
        }
    }
    
    // 문자열이 잘렸다면 말줄임표 추가
    if (src[i] && last_safe_width <= max_width - 3) {
        // 말줄임표를 위한 공간이 있으면 추가
        j = last_safe_pos;
        dest[j++] = '.';
        dest[j++] = '.';
        dest[j++] = '.';
    }
    
    dest[j] = '\0';
}

/* ========== 일정 관리 함수들 ========== */

// 첫 번째 일정 제목 가져오기 (정렬된 순서로)
void get_first_schedule(int year, int month, int day, char *title) {
    if (title == NULL) return;	// title이 NULL이 아닌지 확인
    
    FILE *file = fopen("schedules.txt", "r");
    if (!file) {
        title[0] = '\0';  // 명시적으로 빈 문자열로 설정
        return;
    }
    
    char line[200];
    char schedules[10][50];  // 최대 10개 일정 임시 저장
    int count = 0;
    
    // 해당 날짜의 모든 일정 수집
    while (fgets(line, sizeof(line), file) && count < 10) {
        int s_year, s_month, s_day;
        char s_title[50];
        
        if (sscanf(line, "%d,%d,%d,%49[^\r\n]", &s_year, &s_month, &s_day, s_title) == 4) {
            if (s_year == year && s_month == month && s_day == day) {
                safe_strcpy(schedules[count], s_title, 50);
                count++;
            }
        }
    }
    
    fclose(file);
    
    if (count == 0) {
        strcpy(title, "");
        return;
    }
    
    // 제목순으로 정렬 (간단한 버블 정렬)
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (strcmp(schedules[i], schedules[j]) > 0) {
                char temp[50];
                strcpy(temp, schedules[i]);
                strcpy(schedules[i], schedules[j]);
                strcpy(schedules[j], temp);
            }
        }
    }
    
    // 첫 번째 일정 제목을 셀 크기에 맞게 자르기
    truncate_to_width(schedules[0], title, CELL_WIDTH - 5);
}

/* ========== 캘린더 출력 함수들 ========== */

// 고정 길이 셀 출력 함수
void print_fixed_cell(const char *content) {
	if(content == NULL) {
		content = "";	// NULL을 빈 문자열로 대체
	}
    	int content_width = get_string_width(content);
    	int spaces_needed = CELL_WIDTH - content_width;
    
    	printf("%s", content);
    	for (int i = 0; i < spaces_needed; i++) {
        	printf(" ");
    	}
}

// 캘린더 메인 출력 함수
void print_calendar(int year, int month) {
    char month_year_str[100];
    // 제목 포매팅을 좀 더 정확하게 수정하여 가운데 정렬 느낌을 줍니다.
    sprintf(month_year_str, "                                     %d년 %d월", year, month);

    int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (is_leap_year(year)) {
        days_in_month[2] = 29;
    }

    // 달력의 각 라인 길이를 정확히 맞춥니다.
    printf("┌────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐\n");
    printf("│ %-118s │\n", month_year_str);
    printf("│ %-118s │\n", "일         월         화         수         목         금         토");

    int first_day = get_day_of_week(year, month, 1);
    int num_days = days_in_month[month];
    
    // 빈 줄을 채워 높이를 고정합니다. (총 6주 분량의 라인을 확보)
    int line_count = 0;

    // 첫째 날 시작 위치 맞추기
    for (int i = 0; i < first_day; i++) {
        printf("           ");
    }

    // 날짜 출력
    for (int day = 1; day <= num_days; day++) {
        printf("%-11d", day);
        if ((first_day + day) % 7 == 0) {
            printf("│\n"); // 토요일 뒤에는 파이프 문자와 줄바꿈
            line_count++;
        }
    }

    // 마지막 주가 토요일로 끝나지 않은 경우, 남은 칸을 채우고 마무리
    int total_cells = first_day + num_days;
    if (total_cells % 7 != 0) {
        for (int i = 0; i < (7 - (total_cells % 7)); i++) {
            printf("           ");
        }
        printf("│\n");
        line_count++;
    }

    // 달력이 6줄이 안되면, 빈 줄을 추가하여 높이를 맞춤
    while (line_count < 6) {
        printf("                                                                                                                       │\n");
        line_count++;
    }

    printf("└────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘\n");
}


/* ========== 메인 함수 ========== */

int main(int argc, char *argv[]) {
    // 한글 지원 설정
    setlocale(LC_ALL, "ko_KR.UTF-8");
    
    int year, month;
    
    // 명령행 인자로 년/월 받기, 없으면 현재 날짜
    if (argc >= 3) {
        year = atoi(argv[1]);
        month = atoi(argv[2]);
    } else {
        get_today(&year, &month, NULL);
    }
    
    print_calendar(year, month);
    return 0;
}