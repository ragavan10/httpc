#include <check.h>
#include <stdlib.h>

#include "httpc_err.h"
#include "httpc_types.h"
#include "httpc_request_parser.h"
#include "httpc_str.h"

const struct {
    const char *r;
    httpc_method_e m;
    httpc_str_t p;
    const char **q_keys;
    httpc_str_t *q_vals;
    int qs;
    httpc_version_e v;
    const char **h_keys;
    httpc_str_t *h_vals;
    int hs;
    httpc_str_t *b;
} requests[] = {
    {
        "GET /index.html HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "\r\n",
        GET,
        httpc_str_mk_string("/index.html"),
        NULL,
        NULL,
        0,
        HTTP_1_1,
        (const char *[]){"Host"},
        (httpc_str_t[]) {
            httpc_str_mk_string("www.example.com")
        },
        1,
        NULL
    },
    {
        "GET /search?q=openai+chatgpt HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "User-Agent: Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; Trident/6.0)\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
        "Accept-Language: en-US,en;q=0.5\r\n"
        "Connection: keep-alive\r\n"
        "\r\n",
        GET,
        httpc_str_mk_string("/search"),
        (const char *[]){"q"},
        (httpc_str_t[]) {
            httpc_str_mk_string("openai+chatgpt"),
        },
        1,
        HTTP_1_1,
        (const char  *[]) {
            "Host",
            "User-Agent",
            "Accept",
            "Accept-Language",
            "Connection"
        },
        (httpc_str_t[]) {
            httpc_str_mk_string("www.example.com"),
            httpc_str_mk_string("Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; Trident/6.0)"),
            httpc_str_mk_string("text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"),
            httpc_str_mk_string("en-US,en;q=0.5"),
            httpc_str_mk_string("keep-alive")
        },
        5,
        NULL
    },
    {
        "POST /login HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Content-Length: 27\r\n"
        "\r\n"
        "username=user&password=pass",
        POST,
        httpc_str_mk_string("/login"),
        NULL,
        NULL,
        0,
        HTTP_1_1,
        (const char *[]) {
            "Host",
            "Content-Type",
            "Content-Length"
        },
        (httpc_str_t []){
            httpc_str_mk_string("www.example.com"),
            httpc_str_mk_string("application/x-www-form-urlencoded"),
            httpc_str_mk_string("27")
        },
        3,
        &(httpc_str_t)httpc_str_mk_string("username=user&password=pass")
    },
    {
        "POST /api/v1/resource HTTP/1.1\r\n"
        "Host: api.example.com\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 46\r\n"
        "\r\n"
        "{\"name\":\"John Doe\",\"email\":\"john@example.com\"}",
        POST,
        httpc_str_mk_string("/api/v1/resource"),
        NULL,
        NULL,
        0,
        HTTP_1_1,
        (const char *[]) {
            "Host",
            "Content-Type",
            "Content-Length"
        },
        (httpc_str_t []) {
            httpc_str_mk_string("api.example.com"),
            httpc_str_mk_string("application/json"),
            httpc_str_mk_string("46")
        },
        3,
        &(httpc_str_t)httpc_str_mk_string("{\"name\":\"John Doe\",\"email\":\"john@example.com\"}")
    },
    {
        "GET /path/to/resource/with/a/very/long/url/that/goes/on/and/on?param1=value1&param2=value2&param3=value3 HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "\r\n",
        GET,
        httpc_str_mk_string("/path/to/resource/with/a/very/long/url/that/goes/on/and/on"),
        (const char *[]) {
            "param1",
            "param2",
            "param3"
        },
        (httpc_str_t[]) {
            httpc_str_mk_string("value1"),
            httpc_str_mk_string("value2"),
            httpc_str_mk_string("value3")
        },
        3,
        HTTP_1_1,
        (const char *[]) {
            "Host"
        },
        (httpc_str_t []) {
            httpc_str_mk_string("www.example.com")
        },
        1,
        NULL
    }

};

START_TEST(test_httpc_request_parser_1)
{
    int n = sizeof(requests)/sizeof(requests[0]);
    for (int i = 0; i < n; i++) {
        httpc_request_t *req = NULL;
        httpc_err_e err = httpc_request_parser(requests[i].r, &req);
        ck_assert(err == HTTPC_ERR_NONE);
        ck_assert(requests[i].m == httpc_request_get_method(req));
        ck_assert(httpc_str_cmp(requests[i].p, *httpc_request_get_path(req)));
        for (int j = 0; j < requests[i].qs; j ++) {
            httpc_str_t *val = httpc_request_get_query(req, requests[i].q_keys[j]);
            ck_assert(httpc_str_cmp(*val, requests[i].q_vals[j]));
        }
        ck_assert(requests[i].v == httpc_request_get_version(req));
        for (int j = 0; j < requests[i].hs; j ++) {
            httpc_str_t val = *httpc_request_get_header(req, requests[i].h_keys[j]);
            ck_assert(httpc_str_cmp(val, requests[i].h_vals[j]));
        }
        if (requests[i].b == NULL) {
            ck_assert(req->data == NULL);
        } else {
            httpc_str_t b = *httpc_request_get_body(req);
            ck_assert(httpc_str_cmp(b, *requests[i].b));
        } 
        httpc_free_request(req);
    }
}
END_TEST

Suite * httpc_request_parser_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("RequestParser");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_httpc_request_parser_1);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = httpc_request_parser_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
