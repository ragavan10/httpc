#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>

#include "httpc_utils.h"
#include "httpc_err.h"
#include "httpc_str.h"

#define str3_cmp(s, c0, c1, c2, c3) \
    (s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3)


static int __httpc_utils_is_valid_integer(httpc_str_t *str);

httpc_err_e httpc_utils_time_now(char *date, size_t len)
{
    time_t now = time(NULL);    
    struct tm tm_now;

    // Get the current time in GMT/UTC
    gmtime_r(&now, &tm_now);

    // Format the date according to HTTP date format
    // Example: "Tue, 15 Nov 1994 08:12:31 GMT"
    size_t wlen = strftime(date, len, "%a, %d %b %Y %H:%M:%S GMT", &tm_now);
    date[wlen] = '\0';
    return HTTPC_ERR_NONE;
}

httpc_err_e httpc_utils_cmp_path(httpc_str_t *a, httpc_str_t *b, 
        httpc_request_path_seg_t **seg, int *num) {
    *num = 0;
    httpc_str_t *a_seg = NULL;
    int num_a = 0;
    httpc_err_e err = __httpc_utils_tokenize_path(a, &a_seg, &num_a);
    if (HTTPC_ERR_NONE != err)
        goto ERROR;

    httpc_str_t *b_seg = NULL;
    int num_b = 0;
    err = __httpc_utils_tokenize_path(b, &b_seg, &num_b);
    if (HTTPC_ERR_NONE != err)
        goto ERROR;

    if (num_a != num_b) {
        err = HTTPC_ERR_INVALID_PATH;
        goto ERROR;
    }
    httpc_request_path_seg_t *_seg = NULL;
    httpc_str_t *key = NULL;
    httpc_str_t *type = NULL;
    void *val = NULL;
    for (int i = 0; i < num_a; i ++) {
        if (httpc_str_cmp(a_seg[i], b_seg[i]))
                continue;
        if (!httpc_str_cmp(a_seg[i], b_seg[i]) && b_seg[i].str[0] == '<' && 
                b_seg[i].str[b_seg[i].len - 1] == '>') {
            if (NULL == _seg) {
                _seg = malloc(sizeof(*_seg));
                if (NULL == _seg) {
                    err = HTTPC_ERR_MEM_ALLOC;
                    goto ERROR;
                }
            } else {
                httpc_request_path_seg_t *tmp = realloc(_seg, (*num + 1) * sizeof(*tmp));
                if (NULL == tmp) {
                    err = HTTPC_ERR_MEM_ALLOC;
                    goto ERROR;
                }
                _seg = tmp;
            }
            int j = 1;

            unsigned char *s = &b_seg[i].str[1];
            if (str3_cmp(s, 'i', 'n', 't', ':')) {
                httpc_str_t _val = { 
                    .str = &a_seg[i].str[0],
                    .len = a_seg[i].len
                };
                int int_val = __httpc_utils_is_valid_integer(&_val);
                if (int_val == -1) {
                    err = HTTPC_ERR_INVALID_PATH;
                    goto ERROR;
                }
                key = malloc(sizeof(*key));
                if (NULL == key) {
                    err = HTTPC_ERR_MEM_ALLOC;
                    goto ERROR;
                }

                type = malloc(sizeof(*type));
                if (NULL == type) {
                    err = HTTPC_ERR_MEM_ALLOC;
                    goto ERROR;
                }
                val = malloc(sizeof(int));
                if (NULL == type) {
                    err = HTTPC_ERR_MEM_ALLOC;
                    goto ERROR;
                }
                key->str = &b_seg[i].str[5];
                key->len = b_seg[i].len - 6;
                type->str = (unsigned char *)"int";
                type->len = 3;
                memcpy(val, &int_val, sizeof(int_val));
                _seg[(*num)].key = key;
                _seg[(*num)].type = type;
                _seg[(*num)].val = val;

            } else if (str3_cmp(s, 's', 't', 'r', ':')) {
                key = malloc(sizeof(*key));
                if (NULL == key) {
                    err = HTTPC_ERR_MEM_ALLOC;
                    goto ERROR;
                }
                type = malloc(sizeof(*type));
                if (NULL == type) {
                    err = HTTPC_ERR_MEM_ALLOC;
                    goto ERROR;
                }
                val = malloc(a_seg[i].len + 1);
                if (NULL == type) {
                    err = HTTPC_ERR_MEM_ALLOC;
                    goto ERROR;
                }
                key->str = &b_seg[i].str[5];
                key->len = b_seg[i].len - 6;
                type->str = (unsigned char *)"str";
                type->len = 3;
                memcpy(val, &a_seg[i].str[0], a_seg[i].len);
                *(char *)(val + a_seg[i].len) = '\0';
        
                _seg[(*num)].key = key;
                _seg[(*num)].type = type;
                _seg[(*num)].val = val;
            } else {
                err = HTTPC_ERR_INVALID_PATH;
                goto ERROR;
            }
            (*num) ++;

        } else {
            err = HTTPC_ERR_INVALID_PATH;
            goto ERROR;
        }
    }
    if (a_seg)
        free(a_seg);
    if (b_seg)
        free(b_seg);
    *seg = _seg;
    return HTTPC_ERR_NONE;
ERROR:
    if (a_seg)
        free(a_seg);
    if (b_seg)
        free(b_seg);
    if (_seg) {
        for (int i = 0; i < *num; i ++) {
            free(_seg[i].key);
            free(_seg[i].val);
            free(_seg[i].type);
        }
        free(_seg);
    }
    return err;
}

bool httpc_utils_is_alnum(httpc_str_t *str)
{
    size_t  i = 0;
    while (i < str->len) {
        if ((str->str[i] >= '0' && str->str[i] <= '9') ||
                (str->str[i] >= 'A' && str->str[i] <= 'Z') ||
                (str->str[i] >= 'a' &&  str->str[i] <= 'z')) {
            i ++;
            continue;
        }
        return false;
    }
    return true;
}
httpc_err_e __httpc_utils_tokenize_path(httpc_str_t *s, httpc_str_t **tokens,
        int *num)
{
    *num = 0;
    *tokens = NULL;
    httpc_str_t *_tokens = NULL;
    httpc_err_e err = HTTPC_ERR_NONE;
    int i = 0;
    if (s[0].str[0] != '/') {
        err = HTTPC_ERR_INVALID_PATH;
        goto ERROR;
    }
    unsigned char *start = &s->str[i];
    while (i <= s->len) {
        if (s->str[i] == '/' || i == s->len) {
            if (i == 0) {
                i ++;
                continue;
            }
            if (NULL == _tokens) {
                _tokens = malloc(sizeof(*_tokens));
                if (NULL == _tokens) {
                    err = HTTPC_ERR_MEM_ALLOC;
                    goto ERROR;
                }
            } else {
                httpc_str_t *tmp = realloc(_tokens, (*num + 1) * sizeof(*tmp));
                if (NULL == tmp) {
                    err = HTTPC_ERR_MEM_ALLOC;
                    goto ERROR;
                }
                _tokens = tmp;
            }
            _tokens[(*num)].str = start + 1;
            _tokens[(*num)].len = &s->str[i - 1] - start;
            start = &s->str[i];
            (*num) ++;
        }
        i ++;
    }
    *tokens = _tokens;
    return err;
ERROR:
    if (_tokens)
        free(_tokens);
    *num = 0;
    *tokens = NULL;
    return err;
}


int __httpc_utils_is_valid_integer(httpc_str_t *str)
{
    size_t i = 0;
    int num = 0;
    while (i < str->len) {
        if (str->str[i] > 48 || str->str[i] < 57) {
            num += ((str->str[i] - 48) * pow(10, str->len - i - 1));
        } else {
            return -1;
        }
        i ++;
    }
    return num;
}
