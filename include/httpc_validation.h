#ifndef __HTTPC_VALIDATION_H__
#define __HTTPC_VALIDATION_H__

#include <stdbool.h>
#include "httpc_str.h"

bool httpc_validate_header_host(httpc_str_t *host);
bool httpc_validate_header_accept(httpc_str_t *ac);
bool httpc_validate_header_encoding(httpc_str_t *enc);
bool httpc_validate_header_language(httpc_str_t *lang);
bool httpc_validate_header_connection(httpc_str_t *con);
bool httpc_validate_header_content_type(httpc_str_t *ct);
bool httpc_validate_header_auth(httpc_str_t *auth);

#endif //__VALIDATION__

