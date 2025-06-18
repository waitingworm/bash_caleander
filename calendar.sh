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
if [ ${CURRENT_MONTH:0:1} = "0" ]; then
	CURRENT_MONTH=${CURRENT_MONTH:1:1}
fi

PROGRAM_DIR=$(dirname "$0")

# ========== 시스템 체크 함수 ==========

# 필요한 프로그램 존재 확인
check_programs() {
	if [ ! -f "$PROGRAM_DIR/calendar_system" ] || [ ! -f "$PROGRAM_DIR/calendar_schedule" ]; then
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
	echo "│                                                                      🗓️  터미널 캘린더                                                                                │"
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
│  8. 💬 채팅                          │
│  9. ⏰ 뽀모도로 타이머               │
│  0. 🚪 종료                          │
└──────────────────────────────────────┘
EOF
    
	# 일정 정보 업데이트
	if [ -f "$PROGRAM_DIR/calendar_schedule" ]; then
		SCHEDULE_OUTPUT=$($PROGRAM_DIR/calendar_schedule show $CURRENT_YEAR $CURRENT_MONTH 2>/dev/null)
		SCHEDULE_RESULT=$?
      
      		if [ $SCHEDULE_RESULT -eq 0 ] && [ ! -z "$SCHEDULE_OUTPUT" ]; then
			# 일정이 있는 경우 - 실제 일정 내용을 파싱하여 표시
		            
			# 헤더 라인을 제거하고 실제 일정만 추출
			echo "$SCHEDULE_OUTPUT" | grep "일:" > /tmp/schedule_raw.txt
            
			# 최대 8개 일정을 메뉴박스에 표시할 수 있음
			LINE_COUNT=1
			while read -r schedule_line && [ $LINE_COUNT -le 8 ]; do
				if [ ! -z "$schedule_line" ]; then
					# 일정 라인을 메뉴박스 형식에 맞게 변환
					# "일: 제목" 형식을 " 일일: 제목" 형식으로 변환하고 38자로 맞춤
					FORMATTED_LINE=$(echo "$schedule_line" | sed 's/^ *\([0-9]\+\)일: \(.*\)/ \1일: \2/')
	
					# 38자 길이로 맞추기 (한글 고려하여 간단하게 처리)
					if [ ${#FORMATTED_LINE} -gt 36 ]; then
						FORMATTED_LINE=$(echo "$FORMATTED_LINE" | cut -c1-33)...
					fi
                    
					# 남은 공간을 공백으로 채우기
					# 화면 너비를 정확히 계산
					# 가-힣 : 문자 너비 2, 이모티콘 : 문자 너비 2, 영문/숫자/기호 : 문자 너비 1 -> 캘린더 레이아웃 유지를 위해 설
					REAL_WIDTH=$(echo "$FORMATTED_LINE" | awk 'BEGIN{w=0} {for(i=1;i<=length($0);i++){c=substr($0,i,1); w+=(c ~ /[가-힣🀄-🧿]/ ? 2 : 1)}} END{print w}')
					SPACES_NEEDED=$((38 - REAL_WIDTH))
					PADDING=""
					for i in $(seq 1 $SPACES_NEEDED); do
						PADDING="$PADDING "
					done
                    
					echo "│$FORMATTED_LINE$PADDING│" >> /tmp/schedule_lines.txt
					LINE_COUNT=$((LINE_COUNT + 1))
				else
					break
                		fi
			done < /tmp/schedule_raw.txt
            
			# 빈 줄로 나머지 채우기
			while [ $LINE_COUNT -le 8 ]; do
				echo "│                                      │" >> /tmp/schedule_lines.txt
				LINE_COUNT=$((LINE_COUNT + 1))
			done
            
			rm -f /tmp/schedule_raw.txt
		else
			# 일정이 없는 경우
			echo "│ 이번 달에는 일정이 없습니다.          │" > /tmp/schedule_lines.txt
			for i in $(seq 2 8); do
				echo "│                                      │" >> /tmp/schedule_lines.txt
			done
		fi
        
        	# 메뉴 박스의 일정 부분 (4-11번째 줄)을 새로운 일정 내용으로 교체
		{
			head -3 /tmp/menu_box.txt  # 헤더 부분
			cat /tmp/schedule_lines.txt  # 일정 부분
			tail -n +12 /tmp/menu_box.txt  # 메뉴 부분
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

# 숫자인지 확인하는 함수
is_number() {
	echo "$1" | grep -q '^[0-9]\+$'
}

# ========== 일정 관리 함수들 ==========

# 일정 추가
add_schedule() {
	echo
	echo -e "${CYAN}${BOLD}=== 일정 추가 ===${RESET}"
	echo

	echo -n "년도 (현재: $CURRENT_YEAR): "
	read year
	if [ -z "$year" ]; then
		year=$CURRENT_YEAR
	fi

	echo -n "월 (현재: $CURRENT_MONTH): "
	read month
	if [ -z "$month" ]; then
		month=$CURRENT_MONTH
	fi

	echo -n "일: "
	read day
    
	echo -n "일정 제목: "
	read title
    
	# 유효성 검사
	if ! is_number "$year" || ! is_number "$month" || ! is_number "$day"; then
		echo -e "${RED}잘못된 날짜 형식입니다. 숫자만 입력하세요.${RESET}"
		echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"
		read
		return
	fi

	if [ $month -lt 1 ] || [ $month -gt 12 ]; then
		echo -e "${RED}월은 1-12 사이의 값이어야 합니다.${RESET}"
		echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"
		read
		return
	fi

	# 일수 검사
	case $month in
		1|3|5|7|8|10|12) max_days=31 ;;
		4|6|9|11) max_days=30 ;;
		2)
			# 윤년 계산
			if [ $((year % 4)) -eq 0 ] && [ $((year % 100)) -ne 0 ] || [ $((year % 400)) -eq 0 ]; then
				max_days=29
			else
				max_days=28
			fi
			;;
	esac

	if [ $day -lt 1 ] || [ $day -gt $max_days ]; then
		echo -e "${RED}일은 1-$max_days 사이의 값이어야 합니다.${RESET}"
		echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"
		read
		return
	fi

	# 일정 추가 실행
	$PROGRAM_DIR/calendar_schedule add $year $month $day "$title"
	echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"
	read
}

# 이번 달 모든 일정 보기
show_all_schedules() {
	echo
	echo -e "${CYAN}${BOLD}=== $CURRENT_YEAR년 $CURRENT_MONTH월 일정 목록 ===${RESET}"
	echo
	$PROGRAM_DIR/calendar_schedule show $CURRENT_YEAR $CURRENT_MONTH
	echo
	echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"
	read
}

# 특정 날짜 일정 보기
show_day_schedules() {
	echo
	echo -e "${CYAN}${BOLD}=== 특정 날짜 일정 보기 ===${RESET}"
	echo

	echo -n "년도 (현재: $CURRENT_YEAR): "
	read year
	if [ -z "$year" ]; then
		year=$CURRENT_YEAR
	fi

	echo -n "월 (현재: $CURRENT_MONTH): "
	read month
	if [ -z "$month" ]; then
		month=$CURRENT_MONTH
	fi

	echo -n "일: "
	read day

	# 유효성 검사
	if ! is_number "$year" || ! is_number "$month" || ! is_number "$day"; then
		echo -e "${RED}잘못된 날짜 형식입니다. 숫자만 입력하세요.${RESET}"
		echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"
		read
		return
	fi

	if [ $month -lt 1 ] || [ $month -gt 12 ]; then
		echo -e "${RED}월은 1-12 사이의 값이어야 합니다.${RESET}"
		echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"
		read
		return
	fi

	# 일수 검사
	case $month in
		1|3|5|7|8|10|12) max_days=31 ;;
		4|6|9|11) max_days=30 ;;
		2)
			# 윤년 계산
			if [ $((year % 4)) -eq 0 ] && [ $((year % 100)) -ne 0 ] || [ $((year % 400)) -eq 0 ]; then
				max_days=29
			else
				max_days=28
			fi
			;;
	esac

	if [ $day -lt 1 ] || [ $day -gt $max_days ]; then
		echo -e "${RED}일은 1-$max_days 사이의 값이어야 합니다.${RESET}"
		echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"
		read
		return
	fi

	echo
	echo -e "${CYAN}${BOLD}=== $year년 $month월 $day일 일정 ===${RESET}"
	echo
	$PROGRAM_DIR/calendar_schedule day $year $month $day
	echo
	echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"
	read
}

# 일정 삭제
delete_schedules() {
	echo
	echo -e "${CYAN}${BOLD}=== 일정 삭제 ===${RESET}"
	echo

	echo -n "년도 (현재: $CURRENT_YEAR): "
	read year
	if [ -z "$year" ]; then
		year=$CURRENT_YEAR
	fi

	echo -n "월 (현재: $CURRENT_MONTH): "
	read month
	if [ -z "$month" ]; then
		month=$CURRENT_MONTH
	fi

	echo -n "일: "
	read day

	# 유효성 검사
	if ! is_number "$year" || ! is_number "$month" || ! is_number "$day"; then
		echo -e "${RED}잘못된 날짜 형식입니다. 숫자만 입력하세요.${RESET}"
		echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"
		read
		return
	fi

	if [ $month -lt 1 ] || [ $month -gt 12 ]; then
		echo -e "${RED}월은 1-12 사이의 값이어야 합니다.${RESET}"
		echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"
		read
		return
	fi

	# 일수 검사
	case $month in
		1|3|5|7|8|10|12) max_days=31 ;;
		4|6|9|11) max_days=30 ;;
		2)
			# 윤년 계산
			if [ $((year % 4)) -eq 0 ] && [ $((year % 100)) -ne 0 ] || [ $((year % 400)) -eq 0 ]; then
				max_days=29
			else
				max_days=28
			fi
			;;
	esac

	if [ $day -lt 1 ] || [ $day -gt $max_days ]; then
		echo -e "${RED}일은 1-$max_days 사이의 값이어야 합니다.${RESET}"
		echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"
		read
		return
	fi

	# 일정 삭제 실행
	$PROGRAM_DIR/calendar_schedule delete $year $month $day
	echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"
	read
}

# 특정 달로 이동
go_to_month() {
	echo
	echo -e "${CYAN}${BOLD}=== 특정 달로 이동 ===${RESET}"
	echo

	echo -n "년도 (현재: $CURRENT_YEAR): "
	read year
	if [ -z "$year" ]; then
		year=$CURRENT_YEAR
	fi

	echo -n "월 (현재: $CURRENT_MONTH): "
	read month
	if [ -z "$month" ]; then
		month=$CURRENT_MONTH
	fi

	# 유효성 검사
	if ! is_number "$year" || ! is_number "$month"; then
		echo -e "${RED}잘못된 날짜 형식입니다. 숫자만 입력하세요.${RESET}"
		echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"
		read
		return
	fi

	if [ $month -lt 1 ] || [ $month -gt 12 ]; then
		echo -e "${RED}월은 1-12 사이의 값이어야 합니다.${RESET}"
		echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"
		read
		return
	fi

	CURRENT_YEAR=$year
	CURRENT_MONTH=$month
}

# 터미널 채팅 실행
run_terminal_chat() {
	echo
	echo -e "${CYAN}${BOLD}=== 채팅 종류 선택 ===${RESET}"
	echo
	echo "1. 기존 터미널 채팅"
	echo "2. 채팅"
	echo "3. FTP 파일 공유"
	read -p "선택: " chat_choice
	case $chat_choice in
		1)
			$PROGRAM_DIR/terminal_chat
			;;
		2)
			echo -e "${CYAN}${BOLD}=== 채팅 서버/클라이언트 선택 ===${RESET}"
			echo
			echo "1. 서버 실행 (클라이언트 자동 실행)"
			echo "2. 클라이언트 실행"
			read -p "선택: " chat_mode
			case $chat_mode in
				1)
					read -p "포트 번호 (기본: 8080): " port
					port=${port:-8080}
					read -p "닉네임: " nickname

					# 포트 사용 중인지 확인
					if lsof -i :$port >/dev/null 2>&1; then
						echo "포트 $port가 이미 사용 중입니다. 이전 프로세스를 종료합니다..."
						sudo fuser -k $port/tcp 2>/dev/null
						sleep 2
					fi

					# 서버를 백그라운드로 실행
					$PROGRAM_DIR/chatserver $port &
					SERVER_PID=$!
					# 서버가 시작될 때까지 잠시 대기
					sleep 2
					# 클라이언트를 백그라운드로 실행
					$PROGRAM_DIR/chatclient "127.0.0.1" $port $nickname &
					CLIENT_PID=$!
					echo "서버와 클라이언트가 백그라운드에서 실행 중입니다."
					echo "채팅을 종료하려면 Ctrl+C를 누르세요."
					# 사용자가 Ctrl+C를 누를 때까지 대기
					trap "kill $SERVER_PID $CLIENT_PID 2>/dev/null; sudo fuser -k $port/tcp 2>/dev/null; exit 0" INT
					wait $CLIENT_PID
					# 클라이언트가 종료되면 서버도 종료
					kill $SERVER_PID 2>/dev/null
					sudo fuser -k $port/tcp 2>/dev/null
					;;
				2)
					read -p "서버 IP (기본: 127.0.0.1): " ip
					ip=${ip:-127.0.0.1}
					read -p "포트 번호 (기본: 8080): " port
					port=${port:-8080}
					read -p "닉네임: " nickname
					$PROGRAM_DIR/chatclient $ip $port $nickname
					;;
				*)
					echo "잘못된 선택입니다."
					;;
			esac
			;;
		3)
			echo -e "${CYAN}${BOLD}=== FTP 서버/클라이언트 선택 ===${RESET}"
			echo
			echo "1. 서버 실행 (클라이언트 자동 실행)"
			echo "2. 클라이언트 실행"
			read -p "선택: " ftp_mode
			case $ftp_mode in
				1)
					read -p "전송할 파일명: " filename
					read -p "포트 번호 (기본: 8080): " port
					port=${port:-8080}

					# 포트 사용 중인지 확인
					if lsof -i :$port >/dev/null 2>&1; then
						echo "포트 $port가 이미 사용 중입니다. 이전 프로세스를 종료합니다..."
						sudo fuser -k $port/tcp 2>/dev/null
						sleep 2
					fi

					# 서버를 백그라운드로 실행
					$PROGRAM_DIR/ftpserver $filename $port &
					SERVER_PID=$!
					# 서버가 시작될 때까지 잠시 대기
					sleep 2
					# 클라이언트를 백그라운드로 실행
					$PROGRAM_DIR/ftpclient "127.0.0.1" $port &
					CLIENT_PID=$!
					echo "서버와 클라이언트가 백그라운드에서 실행 중입니다."
					echo "FTP를 종료하려면 Ctrl+C를 누르세요."
					# 사용자가 Ctrl+C를 누를 때까지 대기
					trap "kill $SERVER_PID $CLIENT_PID 2>/dev/null; sudo fuser -k $port/tcp 2>/dev/null; exit 0" INT
					wait $CLIENT_PID
					# 클라이언트가 종료되면 서버도 종료
					kill $SERVER_PID 2>/dev/null
					sudo fuser -k $port/tcp 2>/dev/null
					;;
				2)
					read -p "서버 IP (기본: 127.0.0.1): " ip
					ip=${ip:-127.0.0.1}
					read -p "포트 번호 (기본: 8080): " port
					port=${port:-8080}
					$PROGRAM_DIR/ftpclient $ip $port
					;;
				*)
					echo "잘못된 선택입니다."
					;;
			esac
			;;
		*)
			echo "잘못된 선택입니다."
			;;
	esac
	echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"
	read
}

# 뽀모도로 타이머 실행
run_pomodoro_timer() {
	echo
	echo -e "${CYAN}${BOLD}=== 뽀모도로 타이머 ===${RESET}"
	echo
	$PROGRAM_DIR/pomodoro_timer
	echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"
	read
}

# ========== 메인 루프 ==========

# 프로그램 체크
check_programs

# 메인 루프
while true; do
	show_main_screen
	read choice

	case $choice in
		1) add_schedule ;;
		2) show_all_schedules ;;
		3) show_day_schedules ;;
		4) delete_schedules ;;
		5)
			if [ $CURRENT_MONTH -eq 1 ]; then
				CURRENT_YEAR=$((CURRENT_YEAR - 1))
				CURRENT_MONTH=12
			else
				CURRENT_MONTH=$((CURRENT_MONTH - 1))
			fi
			;;
		6)
			if [ $CURRENT_MONTH -eq 12 ]; then
				CURRENT_YEAR=$((CURRENT_YEAR + 1))
				CURRENT_MONTH=1
			else
				CURRENT_MONTH=$((CURRENT_MONTH + 1))
			fi
			;;
		7) go_to_month ;;
		8) run_terminal_chat ;;
		9) run_pomodoro_timer ;;
		0) exit 0 ;;
		*)
			echo -e "${RED}잘못된 선택입니다. 0-9 사이의 숫자를 입력하세요.${RESET}"
			echo -e "${YELLOW}Enter를 눌러 계속...${RESET}"
			read
			;;
	esac
done
# [calendar.sh 수정본2.txt 기반 코드 종료]