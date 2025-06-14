# [Makefile 수정본2.txt 기반 코드 시작]
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2

# 실행 파일 (terminal_chat, pomodoro_timer 추가)
TARGETS = calendar_system calendar_schedule terminal_chat pomodoro_timer

# 기본 빌드
all: $(TARGETS)
	@echo "✅ 모든 프로그램 컴파일 완료!"
	@echo "📋 실행: ./calendar.sh"
	@chmod +x calendar.sh
	@touch schedules.txt
	@touch chat_log.txt
	@touch files.txt

# 개별 컴파일
calendar_system: calendar_system.c
	@echo "🔨 calendar_system 컴파일 중..."
	@$(CC) $(CFLAGS) -o calendar_system calendar_system.c

calendar_schedule: calendar_schedule.c
	@echo "🔨 calendar_schedule 컴파일 중..."
	@$(CC) $(CFLAGS) -o calendar_schedule calendar_schedule.c

# 터미널 채팅 프로그램 컴파일 규칙
terminal_chat: terminal_chat.c
	@echo "🔨 terminal_chat 컴파일 중..."
	@$(CC) $(CFLAGS) -o terminal_chat terminal_chat.c

# 뽀모도로 타이머 컴파일 규칙
pomodoro_timer: pomodoro_timer.c
	@echo "🔨 pomodoro_timer 컴파일 중..."
	@$(CC) $(CFLAGS) -o pomodoro_timer pomodoro_timer.c

# 정리
clean:
	@echo "🧹 정리 중..."
	@rm -f $(TARGETS)
	@echo "✅ 정리 완료"

# 완전 정리 (데이터 파일 포함)
distclean: clean
	@rm -f schedules.txt chat_*.txt files.txt temp.txt pomodoro_log_*.txt
	@echo "✅ 모든 파일 정리 완료"

# 샘플 데이터 생성
sample:
	@echo "📝 샘플 데이터 생성 중..."
	@echo "2025,6,2,대통령신기원" > schedules.txt
	@echo "2025,6,5,창홀림" >> schedules.txt
	@echo "2025,6,8,자료구조기말" >> schedules.txt
	@echo "2025,6,15,한글테스트" >> schedules.txt
	@echo "2025,6,20,시스템프로그래밍" >> schedules.txt
	@echo "✅ 샘플 데이터 생성 완료"

# 빠른 테스트
test: all sample
	@echo "🧪 테스트 중..."
	@./calendar_system 2025 6
	@echo ""
	@./calendar_schedule show 2025 6

.PHONY: all clean distclean sample test
# [Makefile 수정본2.txt 기반 코드 종료]