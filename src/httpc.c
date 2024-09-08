#include "httpc.h"

#include "httpc_err.h"
#include "httpc_utils.h"
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "httpc_request_parser.h"
#include "httpc_response.h"
#include "httpc_str.h"
#include "httpc_thread_pool.h"
#include "httpc_types.h"

#define HTTPC_MSG_SIZE 4096

typedef struct enpoint {
	httpc_endpoint_fn efn;
	httpc_method_e m;
	httpc_str_t *p;
} endpoint_t;

typedef struct thread_param {
	httpc_t *cntx;
	const char *rs;
	httpc_err_e ret;
	int socket;
} thread_param_t;

httpc_err_e httpc_register_endpoint(httpc_t *cntx, httpc_endpoint_fn efn,
				    httpc_method_e m, httpc_str_t *p)
{
	if (NULL == cntx->endpoint) {
		cntx->endpoint = malloc(sizeof(endpoint_t));
		if (NULL == cntx->endpoint)
			return HTTPC_ERR_MEM_ALLOC;
	} else {
		for (uint16_t i = 0; i < cntx->num_endpoints; i++) {
			httpc_request_path_seg_t *seg = NULL;
			httpc_str_t *a = NULL;
			httpc_str_t *b = NULL;
			httpc_err_e ret = HTTPC_ERR_DEFAULT_VAL;
			bool cmp = false;
			int num_seg = 0;
			if (httpc_str_contains_char(p, '<')) {
				b = p;
				a = ((endpoint_t *)(cntx->endpoint[i]))->p;
				ret = httpc_utils_cmp_path(a, b, &seg,
							   &num_seg);
			} else if (httpc_str_contains_char(
					   ((endpoint_t *)(cntx->endpoint[i]))
						   ->p,
					   '<')) {
				a = p;
				b = ((endpoint_t *)(cntx->endpoint[i]))->p;
				ret = httpc_utils_cmp_path(a, b, &seg,
							   &num_seg);
			} else {
				cmp = httpc_str_cmp(
					*((endpoint_t *)(cntx->endpoint[i]))->p,
					*p);
			}
			//clean up
			if (num_seg > 0 && seg)
				__httpc_request_path_seg_cleanup(seg, num_seg);
			seg = NULL;
			num_seg = 0;
			if (((endpoint_t *)(cntx->endpoint[i]))->m == m &&
			    ((ret == HTTPC_ERR_NONE ||
			      ret == HTTPC_ERR_DUPLICATE_ENDPOINT) ||
			     cmp))
				return HTTPC_ERR_DUPLICATE_ENDPOINT;
		}

		endpoint_t **tmp = realloc(cntx->endpoint,
					   cntx->num_endpoints * sizeof(**tmp));
		if (NULL == tmp)
			return HTTPC_ERR_MEM_ALLOC;
		cntx->endpoint = (void **)tmp;
	}
	// new endpoint
	endpoint_t *nep = malloc(sizeof(*nep));
	if (NULL == nep)
		return HTTPC_ERR_MEM_ALLOC;
	nep->m = m;
	nep->efn = efn;
	nep->p = p;
	cntx->endpoint[cntx->num_endpoints] = (void *)nep;
	cntx->num_endpoints++;
	return HTTPC_ERR_NONE;
}

void *httpc_process_raw_request(void *arg)
{
	thread_param_t *param = (thread_param_t *)arg;
	httpc_request_t *r;
	httpc_err_e ret = HTTPC_ERR_NONE;
	ret = httpc_request_parser(param->rs, &r);
	if (ret != HTTPC_ERR_NONE) {
		param->ret = ret;
		return NULL;
	}
	httpc_response_t res;
	bool enp_found = false;
	for (uint16_t i = 0; i < param->cntx->num_endpoints; i++) {
		httpc_request_path_seg_t *seg = NULL;
		int num_seg = 0;
		bool cmp = false;
		if (httpc_str_contains_char(
			    ((endpoint_t *)(param->cntx->endpoint[i]))->p,
			    '<')) {
			ret = httpc_utils_cmp_path(
				r->request_line->path,
				((endpoint_t *)(param->cntx->endpoint[i]))->p,
				&seg, &num_seg);
			if (seg && num_seg > 0 && ret == HTTPC_ERR_NONE)
				cmp = true;
		} else {
			cmp = httpc_str_cmp(
				*((endpoint_t *)(param->cntx->endpoint[i]))->p,
				*r->request_line->path);
		}
		if (r->request_line->method ==
			    ((endpoint_t *)(param->cntx->endpoint[i]))->m &&
		    cmp) {
			ret = ((endpoint_t *)(param->cntx->endpoint[i]))
				      ->efn(r, seg, num_seg, &res);
			__httpc_request_path_seg_cleanup(seg, num_seg);
			enp_found = true;
			if (ret != HTTPC_ERR_NONE) {
				param->ret = ret;
				return NULL;
			}
		}
	}
	httpc_free_request(r);
	// No endpoint found
	if (!enp_found) {
		httpc_response_init(&res);
		httpc_response_finalize(&res, 502);
	}
	httpc_str_print(*res.rstr);
	printf("\n");
	send(param->socket, res.rstr->str, res.rstr->len, 0);
	close(param->socket);
	free(param);
	return NULL;
}

httpc_err_e httpc_init(httpc_t *cntx, uint16_t port, int num_workers)
{
	struct sockaddr_in *address = malloc(sizeof(*address));
	if (NULL == address)
		return HTTPC_ERR_MEM_ALLOC;
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		printf("%s\n", strerror(errno));
		return HTTPC_ERR_SOCKET;
	}
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
		       &reuse, sizeof(reuse))) {
		printf("%s\n", strerror(errno));
		return HTTPC_ERR_SOCKET;
	}

	address->sin_family = AF_INET;
	address->sin_addr.s_addr = INADDR_ANY;
	address->sin_port = htons(port);

	// Forcefully attaching socket to the port
	if (bind(server_fd, (struct sockaddr *)address,
		 sizeof(struct sockaddr_in)) < 0) {
		printf("%s\n", strerror(errno));
		return HTTPC_ERR_PORT_IN_USE;
	}
	if (listen(server_fd, 3) < 0) {
		printf("%s\n", strerror(errno));
		return HTTPC_ERR_SOCKET_LISTEN;
	}
	// Let's create a thread pool
	httpc_thread_pool_t *tp = NULL;
	if (httpc_thread_pool_create(&tp, num_workers) != HTTPC_ERR_NONE) {
		printf("thread_pool_create\n");
		return HTTPC_ERR_THREAD_CREATE;
	}
	cntx->address = address;
	cntx->tp = tp;
	cntx->endpoint = NULL;
	cntx->num_endpoints = 0;
	cntx->fd = server_fd;
	return HTTPC_ERR_NONE;
}

httpc_err_e httpc_run(httpc_t *cntx)
{
	int new_socket = 0;
	socklen_t addrlen = sizeof(struct sockaddr_in);
#ifdef TEST
	int count = 0;
#endif
	while (1) {
		if ((new_socket = accept(cntx->fd,
					 (struct sockaddr *)cntx->address,
					 &addrlen)) < 0) {
			printf("%s\n", strerror(errno));
			return HTTPC_ERR_SOCKET_ACCEPT;
		}
		char buffer[HTTPC_MSG_SIZE];
		size_t len = read(new_socket, buffer, HTTPC_MSG_SIZE);
		buffer[len] = '\0';
		httpc_response_t *res = NULL;
		thread_param_t *param = malloc(sizeof(*param));
		param->cntx = cntx;
		param->rs = buffer;
		param->socket = new_socket;
		param->ret = HTTPC_ERR_NONE;
		httpc_err_e ret = httpc_thread_add_work(
			cntx->tp, httpc_process_raw_request, (void *)param);
		/* if (ret != HTTPC_ERR_NONE) { */
		/* } */
#ifdef TEST
		count++;
		if (count > 2)
			break;
#endif
	}
}

void __httpc_request_path_seg_cleanup(httpc_request_path_seg_t *seg, int num)
{
	if (NULL == seg)
		return;
	for (int i = 0; i < num; i++) {
		if (seg[i].key)
			free(seg[i].key);
		if (seg[i].val)
			free(seg[i].val);
		if (seg[i].type)
			free(seg[i].type);
	}
	free(seg);
}

void httpc_cleanup(httpc_t *cntx)
{
	free(cntx->address);
	httpc_thread_pool_destroy(cntx->tp);
	for (uint16_t i = 0; i < cntx->num_endpoints; i++) {
		free(cntx->endpoint[i]);
	}
	free(cntx->endpoint);
}
