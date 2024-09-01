#ifndef __HTTPC_THREAD_POOL_H__
#define __HTTPC_THREAD_POOL_H__

#include <stddef.h>

#include "httpc_err.h"
#include "httpc_types.h"

httpc_err_e httpc_thread_pool_create(httpc_thread_pool_t **tp, size_t tcount);
httpc_err_e httpc_thread_add_work(httpc_thread_pool_t *tp, httpc_thread_fn fn,
                                  void *p);
void httpc_thread_pool_destroy(httpc_thread_pool_t *tp);
#endif
