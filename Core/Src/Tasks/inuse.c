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
#include "semphr.h"

#include "Common/utils.h"
#include "Common/config.h"
#include "GUI/porting.h"
#include "Tasks/tasks.h"

#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_it.h"
#include "tim.h"

static uint32_t fraction = CFG_DEFAULT_FOAM_TO_WATER_FRACTION;

static int      using_time   = 0;
static uint32_t pwm_lastTick = 0;

static inuse_information_t inuse_info;
static int                 calc_time  = 0;
static TaskHandle_t        inuse_task = NULL;
static SemaphoreHandle_t   info_mutex = NULL;
static bool                usable     = false;

#define PUMP_ON()  HAL_GPIO_WritePin(GPIO(RELAY1), GPIO_PIN_SET)
#define WATER_ON() HAL_GPIO_WritePin(GPIO(RELAY2), GPIO_PIN_SET)
#define FOAM_ON()  HAL_GPIO_WritePin(GPIO(RELAY3), GPIO_PIN_SET)

#define PUMP_OFF()  HAL_GPIO_WritePin(GPIO(RELAY1), GPIO_PIN_RESET)
#define WATER_OFF() HAL_GPIO_WritePin(GPIO(RELAY2), GPIO_PIN_RESET)
#define FOAM_OFF()  HAL_GPIO_WritePin(GPIO(RELAY3), GPIO_PIN_RESET)

bool inuse = false;

#define BTN1_EVENT (0x01 << 0)
#define BTN2_EVENT (0x01 << 1)
#define BTN3_EVENT (0x01 << 2)
#define BTN4_EVENT (0x01 << 3)
#define BTN7_EVENT (0x01 << 4)
#define BTN8_EVENT (0x01 << 5)

void task_calc_usage() {
    TickType_t       xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(INUSE_SAMPLE_RATE); // 0.1 sec
    bool             already_shown_msgbox = false;

    xLastWakeTime = xTaskGetTickCount();
    for (; inuse;) {
        xSemaphoreTake(info_mutex, portMAX_DELAY);
        if (inuse_info.current_using != CURRENT_USING_NONE) {
            calc_time++;
            if (calc_time == 10) {
                inuse_info.current_used_time++;
                switch (inuse_info.current_using) {
                case CURRENT_USING_WATER:
                    inuse_info.water_used_time++;
                    break;
                case CURRENT_USING_FOAM:
                    inuse_info.foam_used_time++;
                    break;
                default:
                    break;
                }
                calc_time = 0;
            }
            float sample_flow = (inuse_info.current_flow_speed / 1000) *
                                (1000.0f / INUSE_SAMPLE_RATE);
            switch (inuse_info.current_using) {
            case CURRENT_USING_WATER:
                inuse_info.current_usage_water += sample_flow;
                inuse_info.avail -= sample_flow;
                break;
            case CURRENT_USING_FOAM:
                inuse_info.current_usage_foam += sample_flow;
                inuse_info.avail -= sample_flow * (1000.0f / fraction);
                break;
            default:
                break;
            }
            if (inuse_info.avail <= 0.001f) {
                LOG("Avail empty");
                inuse_info.current_using = CURRENT_USING_NONE;
                usable                   = false;
                PUMP_OFF();
                WATER_OFF();
                FOAM_OFF();
                if (!already_shown_msgbox) {
                    show_delay_close_message("余额不足",
                                             "您的剩余余额不足，无法继续用水！",
                                             NULL, 10, NULL);
                    already_shown_msgbox = true;
                }
            } else if (inuse_info.avail <= 10.0f) {
                LOG("Avail too low");
                if (!already_shown_msgbox) {
                    show_delay_close_message("余额警告",
                                             "您的余额仅剩10L，请及时充值。",
                                             NULL, 10, NULL);
                    already_shown_msgbox = true;
                }
            }
        }
        xSemaphoreGive(info_mutex);

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
    if (!usable)
        return;
    xSemaphoreTake(info_mutex, portMAX_DELAY);
    if (inuse_info.current_using == CURRENT_USING_WATER) {
        inuse_info.current_using = CURRENT_USING_NONE;
        PUMP_OFF();
        WATER_OFF();
        FOAM_OFF();
    } else {
        inuse_info.current_using = CURRENT_USING_WATER;
        PUMP_ON();
        WATER_ON();
        FOAM_OFF();
    }
    xSemaphoreGive(info_mutex);
}

static void BTN2_Pressed() {
    if (!usable)
        return;
    xSemaphoreTake(info_mutex, portMAX_DELAY);
    if (inuse_info.current_using == CURRENT_USING_FOAM) {
        inuse_info.current_using = CURRENT_USING_NONE;
        PUMP_OFF();
        WATER_OFF();
        FOAM_OFF();
    } else {
        inuse_info.current_using = CURRENT_USING_FOAM;
        WATER_ON();
        FOAM_ON();
        PUMP_ON();
    }
    xSemaphoreGive(info_mutex);
}

static void BTN3_Pressed() {
    xSemaphoreTake(info_mutex, portMAX_DELAY);
    inuse_info.current_using = CURRENT_USING_NONE;
    xSemaphoreGive(info_mutex);
    PUMP_OFF();
    WATER_OFF();
    FOAM_OFF();
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

    fraction = (uint32_t)GET_CONFIG(CFG_SEL_FOAM_TO_WATER_FRAC);

    info_mutex = xSemaphoreCreateMutex();

    xSemaphoreTake(info_mutex, portMAX_DELAY);
    memset(&inuse_info, 0, sizeof(inuse_information_t));
    strcpy(inuse_info.userId, userId);
    inuse_info.avail = avail;
    if (avail <= 0.001f) {
        usable = false;
    } else {
        usable = true;
    }
    xSemaphoreGive(info_mutex);

    calc_time = 0;
    inuse     = true;
    xTaskCreate(task_calc_usage, "CALC_USAGE", 512, NULL, tskIDLE_PRIORITY + 3,
                NULL);
    xTaskCreate(task_inuse, "INUSE", 256, NULL, tskIDLE_PRIORITY + 3,
                &inuse_task);
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
}

void stop_inuse_task() {
    LOG("[INUSE] Stop.");
    HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_1);
    inuse = false;
    vTaskDelete(inuse_task);
    inuse_task = NULL;

    PUMP_OFF();
    WATER_OFF();
    FOAM_OFF();
    inuse_info.current_using = CURRENT_USING_NONE;
    if (calc_time != 0)
        inuse_info.current_used_time++;
    vSemaphoreDelete(info_mutex);
    info_mutex = NULL;
    memset(&inuse_info, 0, sizeof(inuse_information_t));
    using_time   = 0;
    calc_time    = 0;
    pwm_lastTick = 0;
}

static uint32_t current_used_time = 0;
void            get_inuse_information(inuse_information_t *info) {
    LOG("Get Information...");
    xSemaphoreTake(info_mutex, portMAX_DELAY);
    memcpy(info, &inuse_info, sizeof(inuse_information_t));
    xSemaphoreGive(info_mutex);
}


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
    inuse_info.current_flow_speed = FLOW_SPEED_CALC(freq);
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
