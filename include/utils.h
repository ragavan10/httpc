#ifndef __utils_h__
#define __utils_h__

#include <ctype.h>
#include "httpc.h"

// Macro to check if a character is unreserved
#define HTTPC_UTILS_IS_UNRESERVED(c) (isalnum(c) || (c) == '-' || (c) == '.' || (c) == '_' || (c) == '~')

// Macro to check if a character is a sub-delimiter
#define HTTPC_UTILS_IS_SUB_DELIM(c) ((c) == '!' || (c) == '$' || (c) == '&' || (c) == '\'' || \
                         (c) == '(' || (c) == ')' || (c) == '*' || (c) == '+' || \
                         (c) == ',' || (c) == ';' || (c) == '=')

// Macro to check if a character is valid in a percent-encoded triplet
#define HTTPC_UTILS_IS_HEX_DIGIT(c) (isdigit(c) || (tolower(c) >= 'a' && tolower(c) <= 'f'))

/**
 * Splits a given string into tokens based on the specified delimiter.
 *
 * @param str The input string to be split.
 * @param str_len The length of the input string.
 * @param delimiter The delimiter used to split the string.
 * @param tokens A pointer to an array of strings (tokens) that will be allocated and populated.
 * @param num_tokens A pointer to an integer that will store the number of tokens found.
 * @return An error code indicating success or the type of failure.
 */
httpc_err_e httpc_utils_split_string(const char *str, size_t str_len, char *delimeter, char ***tokens, int *num_tokens);

/**
 * Frees the memory allocated for the tokens.
 *
 * @param tokens The array of tokens to be freed.
 * @param num_tokens The number of tokens in the array.
 */
void httpc_utils_free_tokens(char **tokens, int num_tokens);

/**
 * Parses the request line of an HTTP request.
 *
 * @param req The request line to be parsed.
 * @param req_len The length of the request line.
 * @param request A pointer to a request struct to be populated with the parsed components.
 * @return An error code indicating success or the type of failure.
 */
httpc_err_e httpc_utils_parse_request_line(const char *req, size_t req_len, _request_t *request);

void httpc_utils_free_query(query_t *query);
#endif
