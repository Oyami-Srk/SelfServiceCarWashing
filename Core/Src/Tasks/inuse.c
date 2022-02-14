/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: inuse.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"

#include "Common/utils.h"
#include "GUI/porting.h"
#include "Tasks/tasks.h"

#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_it.h"
#include "tim.h"

static int      using_time   = 0;
static uint32_t pwm_lastTick = 0;

static inuse_information_t inuse_info;
static int                 calc_time  = 0;
static bool                inuse      = false;
TaskHandle_t               inuse_task = NULL;

#define BTN1_EVENT (0x01 << 0)
#define BTN2_EVENT (0x01 << 1)
#define BTN3_EVENT (0x01 << 2)
#define BTN4_EVENT (0x01 << 3)
#define BTN7_EVENT (0x01 << 4)
#define BTN8_EVENT (0x01 << 5)

void task_calc_usage() {
    TickType_t       xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(100); // 0.1 sec

    xLastWakeTime = xTaskGetTickCount();
    for (; inuse;) {
        if (inuse_info.current_using != CURRENT_USING_NONE) {
            calc_time++;
            if (calc_time == 10) {
                inuse_info.current_used_time++;
                calc_time = 0;
            }
            if (inuse_info.current_using == CURRENT_USING_WATER) {
            } else if (inuse_info.current_using == CURRENT_USING_FOAM) {
            }
        }
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
    vTaskDelete(NULL);
}

static bool scan_btn(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    if (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_SET) {
        while (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_SET)
            vTaskDelay(pdMS_TO_TICKS(10));
        return true;
    }
    return false;
}

static void BTN1_Pressed() {
    LOG("[INUSE] Button 1 Pressed.");
    ;
}

static void BTN2_Pressed() {
    LOG("[INUSE] Button 2 Pressed.");
    ;
}

static void BTN3_Pressed() {
    LOG("[INUSE] Button 3 Pressed.");
    ;
}

static void BTN4_Pressed() {
    LOG("[INUSE] Button 4 Pressed.");
    ;
}

static void BTN7_Pressed() {
    LOG("[INUSE] Button 7 Pressed.");
    ;
}

static void BTN8_Pressed() {
    LOG("[INUSE] Button 8 Pressed.");
    ;
}

_Noreturn void task_inuse() {
#define PROCESS_BTN(n)                                                         \
    if (event & n##_EVENT) {                                                   \
        vTaskDelay(pdMS_TO_TICKS(50));                                         \
        if (scan_btn(GPIO(n)))                                                 \
            n##_Pressed();                                                     \
    }                                                                          \
    (void)0

    uint32_t event;

    for (;;) {
        xTaskNotifyWait(0x00, 0xFFFFFFFF, &event, portMAX_DELAY);
        PROCESS_BTN(BTN1);
        PROCESS_BTN(BTN2);
        PROCESS_BTN(BTN3);
        PROCESS_BTN(BTN4);
        PROCESS_BTN(BTN7);
        PROCESS_BTN(BTN8);
        event = 0;
    }
}

void start_inuse_task(const char *userId, float avail) {
    LOG("[INUSE] Start.");
    memset(&inuse_info, 0, sizeof(inuse_information_t));
    calc_time = 0;
    inuse     = true;
    xTaskCreate(task_calc_usage, "CALC_USAGE", 128, NULL, tskIDLE_PRIORITY + 3,
                NULL);
    xTaskCreate(task_calc_usage, "INUSE", 128, NULL, tskIDLE_PRIORITY + 3,
                &inuse_task);
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
}

void stop_inuse_task() {
    LOG("[INUSE] Stop.");
    HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_1);
    inuse = false;
    vTaskDelete(inuse_task);
    inuse_task = NULL;

    HAL_GPIO_WritePin(GPIO(WATER_LED), GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIO(FOAM_LED), GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIO(RELAY8), GPIO_PIN_RESET); // Electromagnet valve
    inuse_info.current_using = CURRENT_USING_NONE;
    if (calc_time != 0)
        inuse_info.current_used_time++;
}

static uint32_t current_used_time = 0;
void            get_inuse_information(inuse_information_t *info) {
    LOG("Get Information...");
    //    memcpy(info, &inuse_info, sizeof(inuse_information_t));
    info->current_used_time = current_used_time;
    current_used_time++;
}

static float calc_flow_speed(float freq) { return (freq + 3) / 8.1f; }

// using for capture the speed
void TIM3_IT_HANDLER() {
    uint32_t currTick = HAL_GetTick();
    if (currTick - pwm_lastTick > 1000) {
        // ignore Freq > 1Hz
        inuse_info.current_flow_speed = 0.0f;
        pwm_lastTick                  = currTick;
        return;
    }
    float freq                    = 1000.0f / (float)(currTick - pwm_lastTick);
    inuse_info.current_flow_speed = calc_flow_speed(freq);
    pwm_lastTick                  = currTick;
}

/*
 * EXTI5 - BTN1
 * EXTI4 - BTN2
 * EXTI2 - BTN3
 * EXTI3 - BTN4
 * EXTI6 - BTN7
 * EXTI7 - BTN8
 */

void EXTI_BTN_IT_HANDLER() {
    if (!inuse || !inuse_task)
        return;
    BaseType_t xHigherPrioTaskWoken;
    uint32_t   ulReturn;
    uint32_t   event = 0;

    ulReturn = taskENTER_CRITICAL_FROM_ISR();
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_5) != RESET)
        event |= BTN1_EVENT;
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_4) != RESET)
        event |= BTN2_EVENT;
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_2) != RESET)
        event |= BTN3_EVENT;
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_3) != RESET)
        event |= BTN4_EVENT;
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_6) != RESET)
        event |= BTN7_EVENT;
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_7) != RESET)
        event |= BTN8_EVENT;
    xTaskNotifyFromISR(inuse_task, event, eSetBits, &xHigherPrioTaskWoken);
    portYIELD_FROM_ISR(xHigherPrioTaskWoken);

    taskEXIT_CRITICAL_FROM_ISR(ulReturn);
}
