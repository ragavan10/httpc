#ifndef __HTTPC_REQUEST_H__
#define __HTTPC_REQUEST_H__

#include "httpc_err.h"
#include "httpc_types.h"

httpc_err_e httpc_request_get_method(httpc_request_t *r, httpc_method_e *m);
httpc_err_e httpc_request_get_query(httpc_request_t *r, const char *key, 
        httpc_str_t *val);


#endif
