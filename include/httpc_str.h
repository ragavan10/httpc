/**
 * @file httpc_str.h
 * @auther Ragavan Kalatharan k.ragavan10@gmail.com
 * @brief String functionalities for httpc library
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HTTPC_STR_H__
#define __HTTPC_STR_H__


#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "httpc_types.h"

#define httpc_str_mk_string(str)        {(unsigned char *)str, sizeof(str) - 1}
#define httpc_str_print(httpc_str)                              \
    do {                                                        \
        printf("%.*s", (int)(httpc_str).len, (httpc_str).str);  \
    } while(0)

#define httpc_str_cmp(httpc_str1, httpc_str2)                   \
    (((httpc_str1).len == (httpc_str2).len) &&                  \
    (memcmp((httpc_str1).str, (httpc_str2).str, (httpc_str1).len) == 0))

#ifdef __cplusplus
}
#endif

#endif  /* __HTTPC_STR_H__ */
