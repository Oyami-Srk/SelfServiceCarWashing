/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCashing
 * File: procedure.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#ifndef __NET_AT_PROCEDURE_H__
#define __NET_AT_PROCEDURE_H__

uint8_t WIFI_CONNECT_TO_AP(uint8_t *buffer, uint16_t len, uint8_t step,
                           Status_t *status);

uint8_t WIFI_UPDATE_TIME(uint8_t *buffer, uint16_t len, uint8_t step,
                         Status_t *status);

uint8_t WIFI_CONNECT_TO_SERVER(uint8_t *buffer, uint16_t len, uint8_t step,
                               Status_t *status);

#endif // __NET_AT_PROCEDURE_H__
