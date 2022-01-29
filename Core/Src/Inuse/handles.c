/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCarWashing
 * File: handles.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#include "main.h"
#include "Common/config.h"
#include "FreeRTOS.h"
#include "tim.h"
#include "cmsis_os2.h"
#include "task.h"

#define SET_GPIO(status, name) HAL_GPIO_WritePin(GPIO(name), GPIO_PIN_##status)


extern float CURRENT_USER_AVAIL_WATER ;
extern float CURRENT_USER_USED_WATER  ;
extern float CURRENT_USER_USED_FOAM   ;
extern float CURRENT_USER_FLOW_SPEED  ;


const osThreadAttr_t task_inuse_attributes = {
    .name       = "inuse",
    .stack_size = 1024,
    .priority   = (osPriority_t)osPriorityAboveNormal,
};

int INUSE_TASK_RUNNING = 0;

_Noreturn void task_inuse(void *argument) {
    for(;INUSE_TASK_RUNNING;) {
        if(HAL_GPIO_ReadPin(GPIO(WATER_BTN)) == GPIO_PIN_SET) {
            vTaskDelay(50);
            if(HAL_GPIO_ReadPin(GPIO(WATER_BTN)) == GPIO_PIN_SET) {
                SET_GPIO(RESET, FOAM_LED);
                SET_GPIO(SET, WATER_LED);
                vTaskDelay(200);
            }
        }

        if(HAL_GPIO_ReadPin(GPIO(FOAM_BTN)) == GPIO_PIN_SET) {
            vTaskDelay(50);
            if(HAL_GPIO_ReadPin(GPIO(FOAM_BTN)) == GPIO_PIN_SET) {
                SET_GPIO(RESET, WATER_LED);
                SET_GPIO(SET, FOAM_LED);
                vTaskDelay(200);
            }
        }


        if(HAL_GPIO_ReadPin(GPIO(BTN3)) == GPIO_PIN_SET) {
            vTaskDelay(50);
            if(HAL_GPIO_ReadPin(GPIO(BTN3)) == GPIO_PIN_SET) {
                SET_GPIO(RESET, WATER_LED);
                SET_GPIO(RESET, FOAM_LED);
                vTaskDelay(200);
            }
        }
    }

    vTaskDelete(NULL);
}

float calculating_water_speed(float freq) {
    return (freq+3)/8.1f;
}

osThreadId_t pid_inuse_task;

void START_INUSE_TASK() {
    INUSE_TASK_RUNNING = 1;
    pid_inuse_task =
        osThreadNew(task_inuse, NULL, &task_inuse_attributes);
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
}

void STOP_INUSE_TASK() {
    CURRENT_USER_FLOW_SPEED = 0;
    CURRENT_USER_USED_WATER = 0;
    CURRENT_USER_AVAIL_WATER = 0;
    CURRENT_USER_USED_FOAM = 0;
//    osThreadTerminate(pid_inuse_task);
    INUSE_TASK_RUNNING = 0;
    HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_1);
    SET_GPIO(RESET, WATER_LED);
    SET_GPIO(RESET, FOAM_LED);
}

int pwm_lastTick = 0;

void PWM_TIMER_ON_RISING() {
    int currTick = HAL_GetTick();
    if(currTick - pwm_lastTick > 1000) {
        // ignore too long freq.
        CURRENT_USER_FLOW_SPEED = 0.0f;
        pwm_lastTick = currTick;
        return;
    }
    float freq = 1000/(currTick - pwm_lastTick);
    CURRENT_USER_FLOW_SPEED = calculating_water_speed(freq);
    pwm_lastTick = currTick;
}
