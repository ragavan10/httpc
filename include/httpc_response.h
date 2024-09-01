#ifndef __HTTPC_RESPONSE_H__
#define __HTTPC_RESPONSE_H__

#include <stdint.h>
#include "httpc_types.h"


httpc_err_e httpc_response_init(httpc_response_t *res);
httpc_err_e httpc_response_set_header(httpc_response_t *res, const char *key,
        const char *val);
httpc_err_e httpc_response_set_status_code(httpc_response_t *res, uint16_t code);
httpc_err_e httpc_response_set_body(httpc_response_t *res, httpc_str_t *body);
httpc_err_e httpc_response_set_json_body(httpc_response_t *res, cJSON *body);

httpc_err_e httpc_response_cleanup(httpc_response_t *res);
httpc_err_e httpc_response_finalize(httpc_response_t *res, 
        uint16_t status_code);
#endif
