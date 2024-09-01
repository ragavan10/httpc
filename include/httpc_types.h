#ifndef __HTTPC_TYPES_H__
#define __HTTPC_TYPES_H__

#include <stddef.h>
#include <stdint.h>
#include <cjson/cJSON.h>

#include "httpc_err.h"

typedef enum httpc_version {
    /// RFC: RFC 2616 (initial), RFC 7230, RFC 7231, RFC 7232, RFC 7233,
    /// RFC 7234, RFC 7235
    HTTP_1_1,
    /// RFC: RFC 7540
    HTTP_2
}httpc_version_e;

typedef enum httpc_method {
    /// RFC 7231, Section 4.3.1
    /// The GET method requests a representation of the specified resource.
    /// Requests using GET should only retrieve data and should have no other
    /// effect.
    GET,
    /// RFC 7231, Section 4.3.3
    /// The POST method requests that the server accept the entity enclosed in
    /// the request as a new subordinate of the resource identified by the URI.
    /// The POST method is used to submit an entity to the specified resource,
    /// often causing a change in state or side effects on the server.
    POST,
    /// RFC 7231, Section 4.3.4
    /// The PUT method requests that the enclosed entity be stored under the 
    /// supplied URI. If the URI refers to an already existing resource, 
    /// it is modified; if the URI does not point to an existing resource, 
    /// then the server can create the resource with that URI.
    PUT,
    /// RFC 7231, Section 4.3.5
    /// The DELETE method deletes the specified resource.
    DELETE,
    /// RFC 7231, Section 4.3.2
    /// The HEAD method is identical to GET except that the server must not
    /// return a message-body in the response. This method can be used for 
    /// obtaining meta-information about the entity implied by the request 
    /// without transferring the entity-body itself.
    HEAD,
    /// RFC 7231, Section 4.3.7
    /// The OPTIONS method is used to describe the communication options for 
    /// the target resource. It allows the client to determine the options 
    /// and/or requirements associated with a resource, or the capabilities 
    /// of a server, without implying a resource action or initiating a 
    /// resource retrieval.
    OPTIONS,
    /// RFC 7231, Section 4.3.8
    /// The TRACE method performs a message loop-back test along the path to
    /// the target resource, providing a useful debugging mechanism. The final 
    /// recipient of the request should reflect the message received back to 
    /// the client as the entity-body of a 200 (OK) response, with a 
    /// Content-Type of "message/http".
    TRACE,
    /// RFC 5789
    /// The PATCH method requests that a set of changes described in the 
    /// request entity be applied to the resource identified by the request 
    /// URI. The set of changes is represented in a format called a 
    /// "patch document."
    PATCH,
    /// RFC 7231, Section 4.3.6
    /// The CONNECT method establishes a tunnel to the server identified by
    /// the target resource. It is primarily used with proxies to establish 
    /// a network tunnel, usually for SSL (HTTPS) connections.
    CONNECT
}httpc_method_e;

typedef enum httpc_auth {
    /// RFC 7617: The 'Basic' HTTP Authentication Scheme
    /// Basic Authentication transmits credentials as username:password
    /// pairs, encoded using Base64.
    AUTH_BASIC,
    /// RFC 6750: The OAuth 2.0 Authorization Framework
    /// Bearer Token Authentication uses tokens issued by an 
    /// authentication server. The token is included in the Authorization 
    /// header of subsequent requests.
    AUTH_BEARER,
    /// RFC 7616: HTTP Digest Access Authentication
    /// Digest Authentication transmits a hashed version of the credentials 
    /// rather than the credentials themselves.
    AUTH_DIGEST,
    /// API Key Authentication involves passing a unique key in the request
    /// header or as a query parameter.
    AUTH_API_KEY,
    /// Custom Authentication allows developers to define their own 
    /// authentication schemes tailored to specific needs.
    AUTH_CUSTOM,

}httpc_auth_e;

typedef enum httpc_header {
    HOST,
    ACCEPT,
    ENCODING,
    LANGUAGE,
    CONNECTION,
    CONTENT_TYPE,
    CONTENT_LENGTH,
    AUTHORIZATION,
} httpc_header_e;



typedef struct httpc_str {
    unsigned char   *str;
    size_t          len;
}httpc_str_t;


typedef struct httpc_key_value_pair {
    httpc_str_t     key;
    httpc_str_t     value;
    struct httpc_key_value_pair *next;
}httpc_key_value_pair_t;


typedef void *(*httpc_thread_fn)(void *param);
typedef struct httpc_work {
    httpc_thread_fn fn;
    void *param;
    struct httpc_work *next;
}httpc_work_t;

typedef struct httpc_thread_pool {
    void *threads;
    void *thread_attrs;
    httpc_work_t *work;
    size_t thread_count;
    void *mutex;
    void *work_cond;
}httpc_thread_pool_t;


/* /// The 'Host' header field */
/* typedef struct httpc_host { */
/*     /// The domain name of the server */
/*     httpc_str_t host; */
/*     /// The TCP port number on which the server is listening */
/*     uint16_t port; */
/* }httpc_host_t; */

/* typedef struct httpc_basic_auth { */
/*     httpc_str_t username; */
/*     httpc_str_t password; */
/* }httpc_basic_auth_t; */

/* typedef struct httpc_digest_auth { */
/*     httpc_str_t username; */
/*     httpc_str_t realm; */
/*     httpc_str_t nonce; */
/*     httpc_str_t uri; */
/*     httpc_str_t response; */
/* }httpc_digest_auth_t; */

/* typedef struct httpc_custom_auth { */
/*     httpc_str_t scheme; */
/*     httpc_str_t credentials; */
/* }httpc_custom_auth_t; */


/* /// The 'Authorization' header field */
/* typedef struct httpc_authorization { */
/*     httpc_auth_e auth_type; */
/*     union { */
/*         httpc_basic_auth_t basic; */
/*         httpc_str_t *bearer; */
/*         httpc_digest_auth_t digest; */
/*         httpc_str_t api_key; */
/*         httpc_custom_auth_t custom; */
/*     }auth_data; */
/* } httpc_authorization_t; */


typedef struct httpc_request_path_seg {
    httpc_str_t *key;
    httpc_str_t *type;
    void *val;
} httpc_request_path_seg_t;

typedef struct httpc_request_line {
    /// HTTP method
    httpc_method_e method;
    /// The specific resource on the server. This is often corresponds to a
    /// file or application endpoint
    httpc_str_t *path;
    /// (optional): A set of key-value pairs providing additional parameters 
    /// to the resource.
    httpc_str_t *query;
    httpc_key_value_pair_t *kv_query;
    /// HTTP version
    httpc_version_e version;
} httpc_request_line_t;


typedef struct httpc_request {
    httpc_request_line_t *request_line;
    httpc_str_t *header;
    httpc_key_value_pair_t *kv_header;
    httpc_str_t  *data;
    cJSON *json;
}httpc_request_t;

typedef struct httpc_response_status_line {
    httpc_version_e version;
    httpc_str_t *status_code;
    httpc_str_t *reason;
}httpc_response_status_line_t;


typedef struct httpc_response {
    httpc_response_status_line_t *status;
    httpc_str_t *header;
    httpc_key_value_pair_t *kv_header;
    httpc_str_t *data;
    httpc_str_t *rstr;
}httpc_response_t;


typedef struct httpc {
    void *address;
    httpc_thread_pool_t *tp;
    void **endpoint;
    uint16_t num_endpoints;
    int fd;
}httpc_t;


typedef httpc_err_e (*httpc_endpoint_fn)(httpc_request_t *r, 
        httpc_response_t *res);

#endif
