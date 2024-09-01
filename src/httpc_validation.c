#include <stdbool.h>

#include "httpc_str.h"
#include "httpc_validation.h"

static bool validate_host(httpc_str_t *host);

bool httpc_validate_header_host(httpc_str_t *host)
{
    return true;
}
bool httpc_validate_header_accept(httpc_str_t *ac)
{
    return true;
}
bool httpc_validate_header_encoding(httpc_str_t *enc)
{
    return true;
}
bool httpc_validate_header_language(httpc_str_t *lang)
{
    return true;
}
bool httpc_validate_header_connection(httpc_str_t *con)
{
    return true;
}
bool httpc_validate_header_content_type(httpc_str_t *ct)
{
    return true;
}
bool httpc_validate_header_auth(httpc_str_t *auth)
{
    return true;
}
