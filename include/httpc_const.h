/**
 * @file httpc_const.h
 * @auther Ragavan Kalatharan k.ragavan10@gmail.com
 * @brief General defines for httpc library
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HTTPC_CONST_H__
#define __HTTPC_CONST_H__

#ifdef __cplusplus
extern "C" {
#endif

/*  SIZES   */
#define HTTPC_MAX_REQUEST_LINE_SIZE                         ( 1 << 13 ) // 8K
#define HTTPC_MAX_HEADER_SIZE                               ( 1 << 13 ) // 8K
#define HTTPC_MAX_BODY_SIZE                                 ( 1 << 21 ) // 2M         

/*  CHARS   */
#define SPACE                                               (unsigned char)' '
#define NL                                                  (unsigned char)'\n'
#define CR                                                  (unsigned char)'\r'
#define NULL_TERM                                           (unsigned char)'\0'
#define COLON                                               (unsigned char)':'
#define QUERY_DILIMETER                                     (unsigned char)'?'
#define EQ                                                  (unsigned char)'='
#define AMP                                                 (unsigned char)'&'

#ifdef __cplusplus
}
#endif

#endif  /* __HTTPC_CONST_H__ */
