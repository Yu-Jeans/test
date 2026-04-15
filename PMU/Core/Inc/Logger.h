/*
 * Logger.h
 *
 *  Created on: 2026. 4. 14.
 *      Author: yujin
 */

#ifndef INC_LOGGER_H_
#define INC_LOGGER_H_

#include <stdio.h>

// 💡 1. 로그 레벨 정의
#define LOG_LEVEL_DEBUG 3 // 시시콜콜한 모든 정보 (함수 진입, 변수값 등)
#define LOG_LEVEL_INFO  2 // 정상 동작 중 주요 상태 변화 (초기화 완료 등)
#define LOG_LEVEL_WARN  1 // 경고 (기본값 세팅 등 시스템은 안 죽는 문제)
#define LOG_LEVEL_ERROR 0 // 치명적 에러 (하드웨어 고장, 시스템 Halt 등)
#define LOG_LEVEL_NONE -1 // 아무것도 출력하지 않음 (출시용)

// 💡 2. 현재 빌드의 로그 레벨 설정 (이 숫자만 바꾸면 전체 시스템의 출력이 변함!)
#define CURRENT_LOG_LEVEL LOG_LEVEL_INFO

// 💡 3. 매크로 함수 구현 (##__VA_ARGS__ 는 printf처럼 가변 인자를 받기 위함)
#if CURRENT_LOG_LEVEL >= LOG_LEVEL_DEBUG
    #define LOG_D(fmt, ...) printf("[DEBUG] " fmt, ##__VA_ARGS__)
#else
    #define LOG_D(fmt, ...) // 코드가 컴파일 단계에서 공중분해됨
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_INFO
    #define LOG_I(fmt, ...) printf("[INFO] " fmt, ##__VA_ARGS__)
#else
    #define LOG_I(fmt, ...)
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_WARN
    #define LOG_W(fmt, ...) printf("\r\n[WARN] " fmt, ##__VA_ARGS__)
#else
    #define LOG_W(fmt, ...)
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_ERROR
    #define LOG_E(fmt, ...) printf("\r\n[ERROR] " fmt, ##__VA_ARGS__)
#else
    #define LOG_E(fmt, ...)
#endif

#endif /* INC_LOGGER_H_ */
