#ifndef __HTTPC_UTILS_H__
#define __HTTPC_UTILS_H__

#include "httpc_err.h"
#include "httpc_types.h"
httpc_err_e httpc_utils_time_now(char *date, size_t len);
httpc_err_e httpc_utils_cmp_path(httpc_str_t *a, httpc_str_t *b, 
        httpc_request_path_seg_t **seg, int *num);

// Internal APIs
httpc_err_e __httpc_utils_tokenize_path(httpc_str_t *s, httpc_str_t **tokens,
        int *num);
#endif
