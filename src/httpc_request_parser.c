#include "httpc_request_parser.h"
#include "httpc_const.h"
#include "httpc_err.h"
#include "httpc_str.h"
#include "httpc_types.h"
#include "httpc_validation.h"

#include <cjson/cJSON.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


#define str3_cmp(s, c0, c1, c2, c3) \
    (s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3)

#define str4_cmp(s, c0, c1, c2, c3, c4) \
    (s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 && s[4] == c4)

#define str6_cmp(s, c0, c1, c2, c3, c4, c5, c6) \
    (s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 && s[4] == c4 && \
     s[5] == c5 && s[6] == c6)
    
#define str7_cmp(s, c0, c1, c2, c3, c4, c5, c6, c7) \
    (s[0] == c0 && s[1] == c1 && s[2] == c2 &&  s[3] == c3 && s[4] == c4 && \
     s[5] == c5 && s[6] == c6 && s[7] == c7)

#define str8_cmp(s, c0, c1, c2, c3, c4, c5, c6, c7, c8) \
    (s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 && s[4] == c4 && \
    s[5] == c5 && s[6] == c6 && s[7] == c7 && s[8] == c8)

#define str9_cmp(s, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9) \
    (s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 && s[4] == c4 && \
    s[5] == c5 && s[6] == c6 && s[7] == c7 && s[8] == c8 && s[9] == c9)

#define str10_cmp(s, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) \
    (s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 && s[4] == c4 && \
    s[5] == c5 && s[6] == c6 && s[7] == c7 && s[8] == c8 && s[9] == c9 && \
    s[10] == c10)

#define str12_cmp(s, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) \
    (s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 && s[4] == c4 && \
    s[5] == c5 && s[6] == c6 && s[7] == c7 && s[8] == c8 && s[9] == c9 && \
    s[10] == c10 && s[11] == c11 && s[12] == c12)

#define str13_cmp(s, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) \
    (s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 && s[4] == c4 && \
    s[5] == c5 && s[6] == c6 && s[7] == c7 && s[8] == c8 && s[9] == c9 && \
    s[10] == c10 && s[11] == c11 && s[12] == c12 && s[13] == c13)

#define str14_cmp(s, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) \
    (s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 && s[4] == c4 && \
    s[5] == c5 && s[6] == c6 && s[7] == c7 && s[8] == c8 && s[9] == c9 && \
    s[10] == c10 && s[11] == c11 && s[12] == c12 && s[13] == c13 && s[14] == c14)


static httpc_err_e __httpc_request_parser(httpc_str_t *r, 
        httpc_method_e *m, httpc_str_t **p, httpc_str_t **q, httpc_version_e *v, 
        httpc_str_t **h, httpc_str_t **b);

static httpc_err_e __httpc_parse_headers(httpc_str_t *hs, 
        httpc_key_value_pair_t **h);

static httpc_err_e __httpc_parse_query(httpc_str_t *qs, 
        httpc_key_value_pair_t **q);

httpc_err_e httpc_request_parser(const char *request, httpc_request_t **r)
{
    httpc_str_t rs = {
        .str = (unsigned char *)request,
        .len = strlen(request)
    };
    // Memory allocation start
    *r = malloc(sizeof(**r));
    if (NULL == *r)
        return HTTPC_ERR_MEM_ALLOC;
    httpc_request_line_t *rl = malloc(sizeof(*rl));
    if (NULL == rl)
        return HTTPC_ERR_MEM_ALLOC;
    httpc_method_e m;
    httpc_version_e v;
    httpc_str_t *h = NULL;
    httpc_str_t *b = NULL;
    httpc_str_t *p = NULL;
    httpc_str_t *q = NULL;
    // First level parsing
    httpc_err_e ret = __httpc_request_parser(&rs, &m, &p, &q, 
            &v, &h, &b);
    if (ret != HTTPC_ERR_NONE)
        return ret;
    if (NULL == p)
        return HTTPC_ERR_INVALID_PATH;
    if (NULL == h)
        return HTTPC_ERR_INVALID_HEADER;
    // Second level parsing
    httpc_key_value_pair_t *qkv = NULL;
    ret = __httpc_parse_query(q, &qkv);
    if (ret != HTTPC_ERR_NONE)
        return ret;

    httpc_key_value_pair_t *hkv = NULL;
    ret = __httpc_parse_headers(h, &hkv);
    if (ret != HTTPC_ERR_NONE)
        return ret;

    // Assemble the struct
    // Assemble request line
    rl->method = m;
    rl->version = v;
    rl->path = p;
    rl->query = q;
    rl->kv_query = qkv;
    // Assemble request
    (*r)->request_line = rl;
    (*r)->header = h;
    (*r)->kv_header = hkv;
    (*r)->data = b;
    (*r)->json = NULL;

    return HTTPC_ERR_NONE;
}

httpc_method_e httpc_request_get_method(const httpc_request_t *r)
{
    return r->request_line->method;
}

httpc_version_e httpc_request_get_version(const httpc_request_t *r)
{
    return r->request_line->version;
}

httpc_str_t *httpc_request_get_path(const httpc_request_t *r)
{
    return r->request_line->path;
}

httpc_str_t *httpc_request_get_query(const httpc_request_t *r, const char *query)
{
    httpc_key_value_pair_t *kv = r->request_line->kv_query;
    httpc_str_t k = {(unsigned char *)query, strlen(query)};
    while (kv) {
        httpc_str_t key = kv->key;
        if (httpc_str_cmp(key, k)) {
            return &kv->value;
        }
        kv = kv->next;
    }
    return NULL;
}

httpc_str_t *httpc_request_get_header(const httpc_request_t *r, const char *header)
{
    httpc_key_value_pair_t *kv = r->kv_header;
    httpc_str_t k = {(unsigned char *)header, strlen(header)};
    while (kv) {
        httpc_str_t key = kv->key;
        if (httpc_str_cmp(key, k)) {
            return &kv->value;
        }
        kv = kv->next;
    }
    return NULL;
}

httpc_str_t *httpc_request_get_body(const httpc_request_t *r)
{
    return r->data;
}


void httpc_free_request(httpc_request_t *r)
{
    r->request_line->path != NULL ? free(r->request_line->path) : (void)0;
    r->request_line->query != NULL ? free(r->request_line->query) : (void)0;
    httpc_key_value_pair_t *qkv = r->request_line->kv_query;
    while (qkv) {
        httpc_key_value_pair_t *tmp = qkv;
        qkv = qkv->next;
        free(tmp);
    }
    free(r->request_line);
    r->header != NULL ? free(r->header) : (void)0;
    httpc_key_value_pair_t *hkv = r->kv_header;
    while (hkv) {
        httpc_key_value_pair_t *tmp = hkv;
        hkv = hkv->next;
        free(tmp);
    }
    r->data != NULL ? free(r->data) : (void)0;
    r->json != NULL ? cJSON_Delete(r->json) : (void)0;
    free(r);

}

static httpc_err_e __httpc_request_parser(httpc_str_t *r, 
        httpc_method_e *m, httpc_str_t **p, httpc_str_t **q, httpc_version_e *v, 
        httpc_str_t **h, httpc_str_t **b)
{
    enum request_fields {
        request_start,
        request_line,
        request_header,
        request_body,
    };

    enum request_line_fields {
        request_method,
        request_path,
        request_query,
        request_version,
    };
    bool request_path_state_completed = false;
    bool request_query_state_completed = false;
    bool request_header_state_completed = false;
    bool request_body_state_completed = false;

    int i = 0;
    unsigned char *request = r->str;
    size_t len = r->len;
    unsigned char *start = request;
    enum request_fields state = request_start;
    enum request_line_fields line_state = request_method;

    while (request[i] != '\0') {
        switch (state) {
            case request_start:
                if (request[i] < 'A' || request[i] > 'Z')
                    return -1;
                state = request_line;
                break;
            case request_line:
                switch (line_state) {
                    case request_method:
                        switch (request + i - start) {
                            // Check for request type
                            case 3:
                                if (str3_cmp(start, 'G', 'E', 'T', ' ')) {
                                    line_state = request_path;
                                    *m = GET;
                                    start = request + i + 1;
                                }
                                else if (str3_cmp(start, 'P', 'U', 'T', ' ')) {
                                    line_state = request_path;
                                    *m = PUT;
                                    start = request + i + 1;
                                }
                                break;
                            case 4:
                                if (str4_cmp(start, 'P', 'O', 'S', 'T', ' ')) {
                                    line_state = request_path;
                                    *m = POST;
                                    start = request + i + 1;
                                } 
                                break;
                            case 6:
                                if (str6_cmp(start, 'D', 'E', 'L', 'E', 'T', 
                                            'E', ' ')) {
                                    line_state = request_path;
                                    *m = DELETE;
                                    start = request + i + 1;
                                }
                                break;
                            /* default: */
                            /*     return HTTPC_ERR_INVALID_METHOD; */
                        }
                        break;
                    case request_path:
                        // Request path must start with /
                        if (*start != '/')
                            return HTTPC_ERR_INVALID_PATH;
                        if (request[i] == SPACE || 
                                request[i] == QUERY_DILIMETER) {
                            *p = malloc(sizeof(**p));
                            if (NULL == *p)
                                return HTTPC_ERR_MEM_ALLOC;
                            // Get the request path
                            (*p)->str = start;
                            (*p)->len = request + i - start;
                            line_state = request_version;
                            start = request + i + 1;
                            request_path_state_completed = true;
                            line_state = (request[i] == SPACE) ? 
                                request_version : request_query;
                        }
                        break;
                    case request_query:
                        if (*(start - 1) != QUERY_DILIMETER)
                            return HTTPC_ERR_INVALID_QUERY;
                        if (request[i] == SPACE) {
                            *q = malloc(sizeof(**q));
                            if (NULL == *q)
                                return HTTPC_ERR_MEM_ALLOC;
                            (*q)->str = start;
                            (*q)->len = request + i - start;
                            line_state = request_version;
                            start = request + i + 1;
                            request_query_state_completed = true;
                        }
                        break;
                    case request_version:
                        switch (request + i - start) {
                            case 7:
                                if (str7_cmp(start, 'H', 'T', 'T','P' , '/', 
                                            '2', CR, NL)) {
                                    *v = HTTP_2;
                                    state = request_header;
                                    start = request + i + 1;
                                }
                                break;
                            case 9:
                                if (str9_cmp(start, 'H', 'T', 'T', 'P', '/', 
                                            '1', '.', '1', CR, NL)) {
                                    *v = HTTP_1_1;
                                    state = request_header;
                                    start = request + i + 1;
                                }
                                break;
                        }
                        break;
                }
                break;
            case request_header:
                if (request[i] == CR && 
                        (i + 1 < len && request[i + 1] == NL) &&
                        (i + 2 < len && request[i + 2] == CR) &&
                        (i + 3 < len && request[i + 3] == NL)) {
                    *h = malloc(sizeof(**h));
                    if (NULL == *h)
                        return HTTPC_ERR_MEM_ALLOC;
                    (*h)->str = start;
                    (*h)->len = request + i - start;
                    if ((*h)->len > HTTPC_MAX_HEADER_SIZE)
                        return HTTPC_ERR_HEADER_TOO_LONG;
                    if (i + 3 < len - 1) {
                        state = request_body;
                    }
                    start = request + i + 4;
                    request_header_state_completed = true;
                }
                break;
            case request_body:
                if (i == len - 1) {
                    *b = malloc(sizeof(**b));
                    if (NULL == *b)
                        return HTTPC_ERR_MEM_ALLOC;
                    (*b)->str = start;
                    (*b)->len = request + i - start + 1;
                    request_body_state_completed = true;
                }
                break;
            default:
                break;
        }
        i ++;

    }
    if (line_state == request_path && request_path_state_completed == false)
        return HTTPC_ERR_INVALID_PATH;
    else if (line_state == request_query && 
            request_query_state_completed == false)
        return HTTPC_ERR_INVALID_QUERY;
    else if (state == request_header && request_header_state_completed == false)
        return HTTPC_ERR_INVALID_HEADER;
    else if (state == request_body && request_body_state_completed == false)
        return HTTPC_ERR_INVALID_BODY;
    return HTTPC_ERR_NONE;
}

static httpc_err_e __httpc_parse_headers(httpc_str_t *hs, 
        httpc_key_value_pair_t **h)
{
    unsigned char *start = hs->str;
    size_t len = hs->len;
    int i = 0;
    unsigned char *cln_pos = NULL;
    bool host_found = false;
    while (i <= len) {
        // colon is found
        if (hs->str[i] == COLON)
            // There is a space between the colon and text
            cln_pos = &hs->str[i];
        // header is found
        if ((hs->str[i] == CR && i + 1 < len && hs->str[i + 1] == NL) || 
                i == len) {
            httpc_key_value_pair_t *hh = malloc(sizeof(*hh));
            if (NULL == hh)
                return HTTPC_ERR_MEM_ALLOC;
            hh->key.str = start;
            hh->key.len = cln_pos - start;
            hh->value.str = cln_pos + 2;
            hh->value.len = &hs->str[i] - cln_pos - 2;
            switch(hh->key.len) {
                case 4:
                    if (str4_cmp(hh->key.str, 'H', 'o', 's', 't', COLON)) { 
                        if (!httpc_validate_header_host(&hh->value))
                            return HTTPC_ERR_INVALID_HEADER;
                        host_found =  true;
                    }
                    break;
                case 6:
                    if (str6_cmp(
                                hh->key.str, 'A', 'c', 'c', 'e', 'p', 't', 
                                COLON) 
                            && !httpc_validate_header_accept(&hh->value))
                        return HTTPC_ERR_INVALID_HEADER;
                    break;
                case 8:
                    if (str8_cmp(
                                hh->key.str, 'E', 'n', 'c', 'o', 'd', 'i', 'n', 
                                'g', COLON) && !httpc_validate_header_encoding(
                                    &hh->value))
                        return HTTPC_ERR_INVALID_HEADER;
                    
                    if (str8_cmp(hh->key.str, 'L', 'a', 'n', 'g', 'u', 'a','g', 
                                'e', COLON) && !httpc_validate_header_language(
                                    &hh->value))
                        return HTTPC_ERR_INVALID_HEADER;
                    break;
                case 10:
                    if (str10_cmp(hh->key.str, 'C', 'o', 'n', 'n', 'e', 'c', 
                                't', 'i', 'o', 'n', COLON) && 
                            !httpc_validate_header_connection(&hh->value))
                        return HTTPC_ERR_INVALID_HEADER;
                    break;
                case 12:
                    if (str12_cmp(hh->key.str, 'C', 'o', 'n', 't', 'e', 'n', 
                                't', '-', 'T', 'y', 'p', 'e', COLON) &&
                            !httpc_validate_header_content_type(&hh->value))
                        return HTTPC_ERR_INVALID_HEADER;
                    break;
                case 13:
                    if (str13_cmp(hh->key.str, 'A', 'u', 't', 'h', 'o', 'r', 
                                'i', 'z', 'a', 't', 'i', 'o', 'n', COLON) &&
                            !httpc_validate_header_auth(&hh->value))
                        return HTTPC_ERR_INVALID_HEADER;
                    break;
                default:
                    break;

            }
            start = &hs->str[i + 2];
            hh->next = *h;
            *h = hh;
        }
        i ++;
    }
    if (host_found == false)
        return HTTPC_ERR_HOST_HEADER_NOT_FOUND;
    return HTTPC_ERR_NONE;
}

static httpc_err_e __httpc_parse_query(httpc_str_t *qs, 
        httpc_key_value_pair_t **q)
{
    if (qs == NULL)
        return HTTPC_ERR_NONE;
    unsigned char *start = qs->str;
    size_t len = qs->len;
    int i = 0;
    unsigned char *eq_pos = NULL;
    while (i <= len) {
        // equal sign is found
        if (qs->str[i] == EQ) {
            eq_pos = &qs->str[i];
        }
        // query string is found 
        if (qs->str[i] == AMP || i == len) {
            httpc_key_value_pair_t *qq = malloc(sizeof(*qq));
            if (NULL == qq)
                return HTTPC_ERR_MEM_ALLOC;
            qq->key.str = start;
            qq->key.len = eq_pos - start;
            qq->value.str = eq_pos + 1;
            qq->value.len = &qs->str[i] - eq_pos - 1;

            qq->next = *q;
            *q = qq;
            start = &qs->str[i + 1];
        }
        i ++;
    }
    return 0;
}
