/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: printf-stdarg.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */

#ifndef __COMMON_PRINTF_STDARG_H__
#define __COMMON_PRINTF_STDARG_H__

int f_printf(const char *format, ...);

int f_sprintf(char *out, const char *format, ...);

int f_snprintf(char *buf, unsigned int count, const char *format, ...);

#endif // __COMMON_PRINTF_STDARG_H__
