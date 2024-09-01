/**
 * @file httpc.h
 * @auther Ragavan Kalatharan k.ragavan10@gmail.com
 * @brief High level APIs for httpc library
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HTTPC_H__
#define __HTTPC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "httpc_err.h"
#include "httpc_types.h"

/**
 * @brief Initializes the HTTP server context.
 */
httpc_err_e httpc_init(httpc_t *cntx, uint16_t port, int num_worker);
httpc_err_e httpc_register_endpoint(httpc_t *cntx, httpc_endpoint_fn efn,
                                    httpc_method_e m, httpc_str_t *p);

httpc_err_e httpc_run(httpc_t *cntx);
void httpc_cleanup(httpc_t *cntx);

void __httpc_request_path_seg_cleanup(httpc_request_path_seg_t *seg, int num);

#ifdef __cplusplus
}
#endif

#endif /* __HTTPC_H__ */
