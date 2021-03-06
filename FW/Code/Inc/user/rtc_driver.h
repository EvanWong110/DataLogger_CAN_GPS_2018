/*
 * precise_time_middleware.h
 *
 *  Created on: 12.11.2017
 *      Author: Michal Kowalik
 */

#ifndef RTC_MIDDLEWARE_H_
#define RTC_MIDDLEWARE_H_

#include "stdint.h"
#include "stdbool.h"
#include "rtc.h"
#include "date_time_data.h"

#define RTC_DRIVER_YEAR_MIN_VALUE	2000
#define RTC_DRIVER_YEAR_MAX_VALUE	2099 //< Difference betweend Year max and min no bigger than 100
#define RTC_DRIVER_MONTH_MIN_VALUE	1
#define RTC_DRIVER_MONTH_MAX_VALUE	12
#define RTC_DRIVER_DAY_MIN_VALUE	1
#define RTC_DRIVER_DAY_MAX_VALUE	31
#define RTC_DRIVER_HOUR_MAX_VALUE	23
#define RTC_DRIVER_MINUTE_MAX_VALUE	59
#define RTC_DRIVER_SECOND_MAX_VALUE	59

typedef enum {
	RTCDriver_Status_OK = 0,
	RTCDriver_Status_NotInitialisedError,
	RTCDriver_Status_NullPointerError,
	RTCDriver_Status_TimeAndDateNotRestoredError,
	RTCDriver_Status_WrongDateFormatError,
	RTCDriver_Status_WrongTimeFormatError,
	RTCDriver_Status_SecondsValueTooHighError,
	RTCDriver_Status_Error
} RTCDriver_Status_TypeDef;

typedef enum {
	RTCDriver_State_UnInitialized = 0,
	RTCDriver_State_Ready
} RTCDriver_State_TypeDef;

static const uint8_t  backupRegistersCount = 3;
static const uint32_t backupRegistersIndexes[] = {0, 10, 12};
static const uint32_t backupRegistersValues[] = {0x5678, 0x1345, 0x5678};


typedef struct {
	RTC_HandleTypeDef*		pRTCHandler;
	RTCDriver_State_TypeDef	state;
} RTCDriver_TypeDef;

RTCDriver_Status_TypeDef RTCDriver_init(RTCDriver_TypeDef* pSelf, RTC_HandleTypeDef* pRTCHandler);
RTCDriver_Status_TypeDef RTCDriver_getDateAndTime(RTCDriver_TypeDef* pSelf, DateTime_TypeDef* pRetDateTime);
RTCDriver_Status_TypeDef RTCDriver_setDateAndTime(RTCDriver_TypeDef* pSelf, DateTime_TypeDef dateTime);

RTCDriver_Status_TypeDef RTCDriver_addSeconds(DateTime_TypeDef* pDateTime, uint32_t seconds);
RTCDriver_Status_TypeDef RTCDriver_substractSeconds(DateTime_TypeDef* pDateTime, uint32_t seconds);

bool RTCDriver_isAfter(DateTime_TypeDef dateTimeExpectedBefore, DateTime_TypeDef dateTimeExpectedAfter);

#endif /* RTC_MIDDLEWARE_H_ */
