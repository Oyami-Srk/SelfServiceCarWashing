/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCashing
 * File: utils.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#ifndef __UTILS_H__
#define __UTILS_H__

#define COMPILE_TIME_ASSERT(expr)    typedef char assert_type[expr ? 1 : -1]
#define STATIC_CHAR_CMP(dyn, static) memcmp(dyn, static, sizeof(static) - 1)

#endif // __UTILS_H__
