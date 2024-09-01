#ifndef __HTTPC_ERR_H__
#define __HTTPC_ERR_H__

typedef enum httpc_err {
    HTTPC_ERR_NONE=0,
    HTTPC_ERR_SOCKET,
    HTTPC_ERR_PORT_IN_USE,
    HTTPC_ERR_SOCKET_LISTEN,
    HTTPC_ERR_SOCKET_ACCEPT,
    HTTPC_ERR_SELECT,
    HTTPC_ERR_SOCKET_READ,
    HTTPC_ERR_THREAD_CREATE,
    HTTPC_ERR_THREAD_JOIN,
    HTTPC_ERR_INVALID_METHOD,
    HTTPC_ERR_INVALID_PATH,
    HTTPC_ERR_INVALID_QUERY,
    HTTPC_ERR_INVALID_VERSION,
    HTTPC_ERR_INVALID_HEADER,
    HTTPC_ERR_HOST_HEADER_NOT_FOUND,
    HTTPC_ERR_INVALID_BODY,
    HTTPC_ERR_HEADER_TOO_LONG,
    HTTPC_ERR_DUPLICATE_ENDPOINT,
    HTTPC_ERR_MEM_ALLOC,
    HTTPC_ERR_OTHER,
}httpc_err_e;

#endif
