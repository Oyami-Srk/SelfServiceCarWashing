/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCashing
 * File: utils.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#include "Net/AT/utils.h"
#include <stdio.h>
#include <string.h>

AT_RESULT AT_GET_RESULT(uint8_t *buffer, uint16_t len) {
    char *p = buffer + len - 1;
    if (*p != '\n')
        return AT_PARSER_ERROR;
    p--;
    int i = len - 1;
    while (*p != '\n') {
        p--;
        i--;
    }
    char pb[16] = {0};
    memcpy(pb, buffer + i, len - i - 2);
    if (strcmp(pb, "OK") == 0) {
        return AT_OK;
    } else if (strcmp(pb, "ERROR") == 0) {
        return AT_ERROR;
    }
}
