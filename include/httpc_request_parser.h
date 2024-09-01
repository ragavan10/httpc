#ifndef __HTTPC_REQUEST_PARSER_H__
#define __HTTPC_REQUEST_PARSER_H__

#include "httpc_err.h"
#include "httpc_types.h"

httpc_err_e httpc_request_parser(const char *request, httpc_request_t **httpc_request);
httpc_method_e httpc_request_get_method(const httpc_request_t *r);
httpc_version_e httpc_request_get_version(const httpc_request_t *r);
httpc_str_t *httpc_request_get_path(const httpc_request_t *r);
httpc_str_t *httpc_request_get_query(const httpc_request_t *r, const char *query);
httpc_str_t *httpc_request_get_header(const httpc_request_t *r, const char *header);
httpc_str_t *httpc_request_get_body(const httpc_request_t *r);
void httpc_free_request(httpc_request_t *r);

#endif
