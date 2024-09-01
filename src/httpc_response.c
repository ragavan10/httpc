#include "httpc_response.h"
#include "httpc_err.h"
#include "httpc_const.h"
#include "httpc_str.h"
#include "httpc_types.h"
#include "httpc_utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

static struct {
    uint16_t code;
    httpc_str_t code_str;
    httpc_str_t reason;
} code_reason_map[] = {
    {200, httpc_str_mk_string("200"), httpc_str_mk_string("OK")},
    {201, httpc_str_mk_string("201"), httpc_str_mk_string("Created")},
    {202, httpc_str_mk_string("202"), httpc_str_mk_string("Accepted")},
    {400, httpc_str_mk_string("400"), httpc_str_mk_string("Bad Request")},
    {401, httpc_str_mk_string("401"), httpc_str_mk_string("Unauthorized")},
    {403, httpc_str_mk_string("403"), httpc_str_mk_string("Forbidden")},
    {404, httpc_str_mk_string("404"), httpc_str_mk_string("Not Found")},
    {500, httpc_str_mk_string("500"), httpc_str_mk_string("Internal Server Error")},
    {501, httpc_str_mk_string("501"), httpc_str_mk_string("Not Implemented")},
    {502, httpc_str_mk_string("502"), httpc_str_mk_string("Bad Gateway")}
};

static struct {
    httpc_version_e version;
    httpc_str_t version_str;
} version_str_map[] = {
    {HTTP_1_1, httpc_str_mk_string("HTTP/1.1")},
    {HTTP_2, httpc_str_mk_string("HTTP/2")}
};

static httpc_err_e httpc_response_gen_status_line(httpc_response_t *res, 
        uint16_t status_code)
{
    res->status = malloc(sizeof(*res->status));
    if (NULL == res->status)
        return HTTPC_ERR_MEM_ALLOC;
    res->status->reason = NULL;
    res->status->version = HTTP_1_1;
    for (int i = 0; i < sizeof(code_reason_map)/sizeof(code_reason_map[i]);
            i ++) {
        if (status_code == code_reason_map[i].code) {
            res->status->status_code = &code_reason_map[i].code_str;
            res->status->reason = &code_reason_map[i].reason;
        }
    }
    return HTTPC_ERR_NONE;
}

httpc_err_e httpc_response_init(httpc_response_t *res)
{
    res->data        = NULL;
    res->header      = NULL;
    res->kv_header   = NULL;
    res->status      = NULL;
    res->rstr        = NULL;
    return HTTPC_ERR_NONE;
}


httpc_err_e httpc_response_set_header(httpc_response_t *res, const char *key,
        const char *val)
{
    size_t start = 0;
    if (NULL == res)
        return HTTPC_ERR_MEM_ALLOC;
    // Check if memory is allocated for res
    if (NULL == res->header) {
        // Allocate memory for header
        res->header = malloc(sizeof(*res->header));
        if (NULL == res->header)
            return HTTPC_ERR_MEM_ALLOC;

        res->header->len = strlen(key) + sizeof(SPACE) + sizeof(COLON) + 
            strlen(val) + sizeof(CR) + sizeof(NL);
        res->header->str = malloc(res->header->len);
        if (NULL == res->header->str)
            return HTTPC_ERR_MEM_ALLOC;
    // Re allocate memory
    } else {
        start = res->header->len;
        size_t len = strlen(key) + sizeof(SPACE) + sizeof(COLON) + 
            strlen(val) + sizeof(CR) + sizeof(NL) + res->header->len;
        unsigned char *tmp = realloc(res->header->str, len);
        if (NULL == tmp)
            return HTTPC_ERR_MEM_ALLOC;
        res->header->len = len;
        res->header->str = tmp;
    }
    // Form the response here
    memcpy(res->header->str + start, key, strlen(key)); 
    memcpy(res->header->str + start + strlen(key), ": ", 2);
    memcpy(res->header->str + start + strlen(key) + 2, val, strlen(val));
    memcpy(res->header->str + start + strlen(key) + 2 + strlen(val), "\r\n", 2);
    /* snprintf((char *)res->header->str + start, res->header->len - start, */ 
    /*         "%s: %s\r\n", key, val); */

    // Allocate memory for kv_header
    httpc_key_value_pair_t *rkv = malloc(sizeof(*rkv));
    if (NULL == rkv)
        return HTTPC_ERR_MEM_ALLOC;
    rkv->key.str = res->header->str + start;
    rkv->key.len = strlen(key);

    rkv->value.str = res->header->str + start + strlen(key) + sizeof(SPACE) + 
        sizeof(COLON) + 1;
    rkv->value.len = strlen(val);
    rkv->next = res->kv_header;
    res->kv_header = rkv;
    
    return HTTPC_ERR_NONE;
}

httpc_err_e httpc_response_set_body(httpc_response_t *res, httpc_str_t *body)
{
    if (NULL == res)
        return HTTPC_ERR_MEM_ALLOC;
    res->data = malloc(sizeof(*res->data));
    if (NULL == res->data)
        return HTTPC_ERR_MEM_ALLOC;
    res->data->str = malloc(body->len);
    if (res->data->str)
        return HTTPC_ERR_MEM_ALLOC;
    memcpy(res->data->str, body->str, res->data->len);
    return HTTPC_ERR_NONE;
}

httpc_err_e httpc_response_finalize(httpc_response_t *res, 
        uint16_t status_code)
{
    int ret = httpc_response_gen_status_line(res, status_code);
    if (ret != HTTPC_ERR_NONE)
        return ret;
    
    ret = httpc_response_set_header(res, "Server", "HTTPC");
    if (ret != HTTPC_ERR_NONE)
        return ret;

    char date[255];
    httpc_utils_time_now(date, sizeof(date));
    ret = httpc_response_set_header(res, "Date", date);
    if (ret != HTTPC_ERR_NONE)
        return ret;

    httpc_key_value_pair_t *rkv = res->kv_header;
    bool ct_found = false;
    while (rkv) {
        httpc_str_t cmp = httpc_str_mk_string("Content-Type");
        if (httpc_str_cmp(cmp, rkv->key)) {
            ct_found = true;
        }
        rkv = rkv->next;
    }
    if (!ct_found) {
        ret = httpc_response_set_header(res, 
                "Content-Type", "text/html; charset=UTF-8");
        if (ret != HTTPC_ERR_NONE)
            return ret;
        ret = httpc_response_set_header(res, "Content-Length", "0");
        if (ret != HTTPC_ERR_NONE)
            return ret;
    }
    ret = httpc_response_set_header(res, "Connection", "Close");
    if (ret != HTTPC_ERR_NONE)
        return ret;
    if (res->data) {
        // 2MiB Max
        char c_str[7];
        snprintf(c_str, sizeof(c_str), "%zu", res->data->len);
        ret = httpc_response_set_header(res, "Content-Length", c_str);
        if (ret != HTTPC_ERR_NONE)
            return ret;
    }
    size_t st_line_len = (version_str_map[0].version_str.len + sizeof(SPACE) +
            res->status->status_code->len + sizeof(SPACE) +
            res->status->reason->len +
            sizeof(CR) + sizeof(NL));
    size_t header_len = res->header->len + sizeof(CR) + sizeof(NL);
    res->rstr = malloc(sizeof(*res->rstr));
    if (NULL == res->rstr)
        return HTTPC_ERR_MEM_ALLOC;
    res->rstr->str = malloc(st_line_len + header_len);
    if (NULL == res->rstr)
        return HTTPC_ERR_MEM_ALLOC;
    res->rstr->len = st_line_len + header_len;
    memcpy(res->rstr->str, version_str_map[0].version_str.str, 
            version_str_map[0].version_str.len);
    memcpy(res->rstr->str + version_str_map[0].version_str.len, 
            " ", sizeof(SPACE));
    memcpy(res->rstr->str + version_str_map[0].version_str.len + 1, 
            res->status->status_code->str, res->status->status_code->len);
    memcpy(res->rstr->str + version_str_map[0].version_str.len + 1 + 
            res->status->status_code->len, " ", sizeof(SPACE));
    memcpy(res->rstr->str + version_str_map[0].version_str.len + 1 + 
            res->status->status_code->len + 1, res->status->reason->str,
            res->status->reason->len);
    memcpy(res->rstr->str + version_str_map[0].version_str.len + 1 + 
            res->status->status_code->len + 1 + res->status->reason->len,
            "\r\n", 2);
    memcpy(res->rstr->str + st_line_len, res->header->str, res->header->len);
    memcpy(res->rstr->str + st_line_len + res->header->len, "\r\n", 2);
    res->rstr->len = st_line_len + header_len;

    if (res->data) {
        res->rstr->len += res->data->len + sizeof(CR) + sizeof(NL); 
        unsigned char *tmp = realloc(res->rstr->str, res->rstr->len);
        if (NULL == tmp)
            return HTTPC_ERR_MEM_ALLOC;
        res->rstr->str = tmp;
        snprintf((char *)(res->rstr->str + st_line_len + header_len), res->data->len,
                "%s\r\n", res->data->str);
    }
    return HTTPC_ERR_NONE;
}




