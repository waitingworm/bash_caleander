#!/bin/bash

# [calendar.sh 수정본2.txt 기반 코드 시작]
# ========== 색상 및 전역 변수 설정 ==========
RED='\033[31m'
GREEN='\033[32m'
YELLOW='\033[33m'
CYAN='\033[36m'
WHITE='\033[37m'
BOLD='\033[1m'
RESET='\033[0m'

# 현재 날짜 (기본값)
CURRENT_YEAR=$(date +%Y)
CURRENT_MONTH=$(date +%m)
# 앞의 0 제거
if [[ ${CURRENT_MONTH:0:1} == "0" ]]; then
    CURRENT_MONTH=${CURRENT_MONTH:1:1}
fi

PROGRAM_DIR=$(dirname "$0")

# ========== 시스템 체크 함수 ==========

# 필요한 프로그램 존재 확인
check_programs() {
    # [message (1).txt 통합을 위해 terminal_chat 확인 로직 추가]
    if [ ! -f "$PROGRAM_DIR/calendar_system" ] || [ ! -f "$PROGRAM_DIR/calendar_schedule" ] || [ ! -f "$PROGRAM_DIR/terminal_chat" ]; then
        echo -e "${RED}필요한 프로그램이 컴파일되지 않았습니다.${RESET}"
        echo "make 명령을 실행하여 컴파일하세요."
        exit 1
    fi
}
# [calendar.sh 수정본2.txt 기반 코드 종료]

# [calendar.sh 수정본2.txt 기반 코드 시작]
# ========== 화면 출력 함수들 ==========

# 메인 화면 출력
show_main_screen() {
    clear
    
    # 상단 제목
    echo "┌──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐"
    echo "│                                                                       🗓️  터미널 캘린더                                                                                │"
    echo "└──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘"
    echo
    
    # 캘린더를 임시 파일로 생성
    $PROGRAM_DIR/calendar_system $CURRENT_YEAR $CURRENT_MONTH > /tmp/calendar_output.txt
    
    # 우측 메뉴 박스 생성
    cat > /tmp/menu_box.txt << 'EOF'
┌──────────────────────────────────────┐
│              이번 달 일정            │
├──────────────────────────────────────┤
│                                      │
│                                      │
│                                      │
│                                      │
│                                      │
│                                      │
│                                      │
│                                      │
├──────────────────────────────────────┤
│                  메뉴                │
├──────────────────────────────────────┤
│  1. ➕ 일정 추가                     │
│  2. 📋 이번 달 모든 일정 보기        │
│  3. 📅 특정 날짜 일정 보기           │
│  4. 🗑️  일정 삭제 (날짜별)            │
│  5. ⬅️  이전 달                       │
│  6. ➡️  다음 달                       │
│  7. 📅 특정 달로 이동                │
│  8. 💬 터미널 채팅                   │
│  9. ⏱️  뽀모도로 타이머              │
│  0. 🚪 종료                          │
└──────────────────────────────────────┘
EOF
    
    # 일정 정보 업데이트
    if [ -f "$PROGRAM_DIR/calendar_schedule" ];
    then
        SCHEDULE_OUTPUT=$($PROGRAM_DIR/calendar_schedule show $CURRENT_YEAR $CURRENT_MONTH 2>/dev/null)
        SCHEDULE_RESULT=$?
        if [ $SCHEDULE_RESULT -eq 0 ] && [ ! -z "$SCHEDULE_OUTPUT" ];
        then
            # 일정이 있는 경우
            echo "$SCHEDULE_OUTPUT" | grep "일:" > /tmp/schedule_raw.txt
            
            LINE_COUNT=1
            while read -r schedule_line && [ $LINE_COUNT -le 8 ];
            do
                if [ ! -z "$schedule_line" ]; then
                    FORMATTED_LINE=$(echo "$schedule_line" | sed 's/^ *\([0-9]\+\)일: \(.*\)/ \1일: \2/')
                    
                    if [ ${#FORMATTED_LINE} -gt 36 ];
                    then
                        FORMATTED_LINE=$(echo "$FORMATTED_LINE" | cut -c1-33)...
                    fi
                    
		    REAL_WIDTH=$(echo "$FORMATTED_LINE" | awk 'BEGIN{w=0} {for(i=1;i<=length($0);i++){c=substr($0,i,1); w+=(c ~ /[가-힣]/ ? 2 : 1)}} END{print w}')
		    SPACES_NEEDED=$((38 - REAL_WIDTH))
                    PADDING=""
                    for i in $(seq 1 $SPACES_NEEDED);
                    do
                        PADDING="$PADDING "
                    done
                    
                    echo "│$FORMATTED_LINE$PADDING│" >> /tmp/schedule_lines.txt
                    LINE_COUNT=$((LINE_COUNT + 1))
                else
                    break
                fi
            done < /tmp/schedule_raw.txt
            
            while [ $LINE_COUNT -le 8 ];
            do
                echo "│                                      │" >> /tmp/schedule_lines.txt
                LINE_COUNT=$((LINE_COUNT + 1))
            done
            
            rm -f /tmp/schedule_raw.txt
        else
            # 일정이 없는 경우
            echo "│ 이번 달에는 일정이 없습니다.          │" > /tmp/schedule_lines.txt
            for i in $(seq 2 8);
            do
                echo "│                                      │" >> /tmp/schedule_lines.txt
            done
        fi
        
        {
            head -3 /tmp/menu_box.txt
            cat /tmp/schedule_lines.txt
            tail -n +12 /tmp/menu_box.txt
        } > /tmp/menu_box_updated.txt
        
        cp /tmp/menu_box_updated.txt /tmp/menu_box.txt
        rm -f /tmp/schedule_lines.txt /tmp/menu_box_updated.txt
    fi
    
    # 캘린더와 메뉴를 좌우로 배치
    paste -d ' ' /tmp/calendar_output.txt /tmp/menu_box.txt
    
    echo
    echo -e "${WHITE}현재: ${BOLD}${CURRENT_YEAR}년 ${CURRENT_MONTH}월${RESET}                                                      선택하세요 (0-9): \c"
    
    # 임시 파일 정리
    rm -f /tmp/calendar_output.txt /tmp/menu_box.txt
}

# ========== 숫자 검증 함수 ==========

is_number() {
    echo "$1" | grep -q '^[0-9]\+$'
}

# ========== 일정 관리 함수들 ==========

add_schedule() {
    echo
    echo -e "${CYAN}${BOLD}=== 일정 추가 ===${RESET}"
    echo
    
    echo -n "년도 (현재: $CURRENT_YEAR): "
    read year
    if [ -z "$year" ]; then year=$CURRENT_YEAR; fi
    
    echo -n "월 (현재: $CURRENT_MONTH): "
    read month
    if [ -z "$month" ]; then month=$CURRENT_MONTH; fi
    
    echo -n "일: "
    read day
    
    echo -n "일정 제목: "
    read title
    
    # 유효성 검사
    if ! is_number "$year" || ! is_number "$month" || ! is_number "$day"; then
        echo -e "${RED}잘못된 날짜 형식입니다. 숫자만 입력하세요.${RESET}"
        echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"; read; return
    fi
    
    if [ $month -lt 1 ] || [ $month -gt 12 ]; then
        echo -e "${RED}월은 1-12 사이의 값이어야 합니다.${RESET}"
        echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"; read; return
    fi
    
    if [ $day -lt 1 ] || [ $day -gt 31 ]; then
        echo -e "${RED}일은 1-31 사이의 값이어야 합니다.${RESET}"
        echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"; read; return
    fi
    
    if [ -z "$title" ]; then
        echo -e "${RED}일정 제목을 입력해야 합니다.${RESET}"
        echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"; read; return
    fi
    
    $PROGRAM_DIR/calendar_schedule add $year $month $day "$title"
    
    echo -e "${GREEN}일정이 추가되었습니다!${RESET}"
    echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"; read
}

show_all_schedules() {
    echo
    echo -e "${CYAN}${BOLD}=== ${CURRENT_YEAR}년 ${CURRENT_MONTH}월 전체 일정 ===${RESET}"
    $PROGRAM_DIR/calendar_schedule show $CURRENT_YEAR $CURRENT_MONTH
    echo
    echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"; read
}

show_day_schedules() {
    echo
    echo -e "${CYAN}${BOLD}=== 특정 날짜 일정 보기 ===${RESET}"
    echo
    
    echo -n "년도 (현재: $CURRENT_YEAR): "
    read year
    if [ -z "$year" ]; then year=$CURRENT_YEAR; fi
    
    echo -n "월 (현재: $CURRENT_MONTH): "
    read month
    if [ -z "$month" ]; then month=$CURRENT_MONTH; fi
    
    echo -n "일: "
    read day
    
    if is_number "$year" && is_number "$month" && is_number "$day"; then
        $PROGRAM_DIR/calendar_schedule day $year $month $day
    else
        echo -e "${RED}잘못된 날짜 형식입니다.${RESET}"
    fi
    
    echo
    echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"; read
}

delete_schedule() {
    echo
    echo -e "${CYAN}${BOLD}=== 일정 삭제 ===${RESET}"
    echo
    
    echo -n "년도 (현재: $CURRENT_YEAR): "
    read year
    if [ -z "$year" ]; then year=$CURRENT_YEAR; fi
    
    echo -n "월 (현재: $CURRENT_MONTH): "
    read month
    if [ -z "$month" ]; then month=$CURRENT_MONTH; fi
    
    echo -n "일: "
    read day
    
    if ! is_number "$year" || ! is_number "$month" || ! is_number "$day"; then
        echo -e "${RED}잘못된 날짜 형식입니다.${RESET}"
        echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"; read; return
    fi
    
    echo
    echo -e "${WHITE}${year}년 ${month}월 ${day}일의 일정을 삭제합니다.${RESET}"
    $PROGRAM_DIR/calendar_schedule delete $year $month $day
    
    echo
    echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"; read
}

# ========== 달력 이동 함수들 ==========

prev_month() {
    CURRENT_MONTH=$((CURRENT_MONTH - 1))
    if [ $CURRENT_MONTH -eq 0 ]; then
        CURRENT_MONTH=12
        CURRENT_YEAR=$((CURRENT_YEAR - 1))
    fi
}

next_month() {
    CURRENT_MONTH=$((CURRENT_MONTH + 1))
    if [ $CURRENT_MONTH -eq 13 ]; then
        CURRENT_MONTH=1
        CURRENT_YEAR=$((CURRENT_YEAR + 1))
    fi
}

goto_month() {
    echo
    echo -e "${CYAN}${BOLD}=== 특정 달로 이동 ===${RESET}"
    echo
    
    echo -n "년도를 입력하세요: "
    read year
    echo -n "월을 입력하세요 (1-12): "
    read month
    
    if is_number "$year" && is_number "$month"; then
        if [ $month -ge 1 ] && [ $month -le 12 ]; then
            CURRENT_YEAR=$year
            CURRENT_MONTH=$month
        else
            echo -e "${RED}월은 1-12 사이의 값이어야 합니다.${RESET}"
            echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"; read
        fi
    else
        echo -e "${RED}잘못된 입력입니다.${RESET}"
        echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"; read
    fi
}
# [calendar.sh 수정본2.txt 기반 코드 종료]

# [message (1).txt 통합을 위해 추가된 코드 시작]
# ========== 추가 기능 함수들 ==========
run_chat_system() {
    clear
    echo -e "${CYAN}${BOLD}=== 💬 터미널 채팅 시스템 시작 ===${RESET}"
    # 컴파일된 채팅 프로그램 실행
    $PROGRAM_DIR/terminal_chat
    echo
    echo -e "${YELLOW}채팅 시스템을 종료했습니다. Enter를 눌러 캘린더로 돌아갑니다...${RESET}"
    read
}

# 뽀모도로 타이머 실행
run_pomodoro_timer() {
    $PROGRAM_DIR/pomodoro_timer
}
# [message (1).txt 통합을 위해 추가된 코드 종료]

# [calendar.sh 수정본2.txt 기반 코드 시작]
# 추가기능 2 (미구현)
additional_feature_2() {
    echo
    echo -e "${CYAN}${BOLD}=== 🔧 추가기능 2 ===${RESET}"
    echo -e "${YELLOW}이 기능은 아직 구현되지 않았습니다.${RESET}"
    echo
    echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"; read
}

# ========== 메인 루프 ==========
main_loop() {
    while true; do
        show_main_screen
        read choice
        
        case $choice in
            1) add_schedule ;;
            2) show_all_schedules ;;
            3) show_day_schedules ;;
            4) delete_schedule ;;
            5) prev_month ;;
            6) next_month ;;
            7) goto_month ;;
            8) run_chat_system ;;
            9) run_pomodoro_timer ;;
            0) 
                echo
                echo -e "${GREEN}터미널 캘린더를 종료합니다. 👋${RESET}"
                exit 0 
                ;;
            *) 
                echo
                echo -e "${RED}잘못된 선택입니다. 0-9 사이의 숫자를 입력하세요.${RESET}"
                echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"; read
                ;;
        esac
    done
}

# ========== 프로그램 시작 ==========
check_programs
main_loop
# [calendar.sh 수정본2.txt 기반 코드 종료]