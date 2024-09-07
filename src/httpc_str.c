#include "httpc_str.h"

bool httpc_str_contains_char(httpc_str_t *s, char c)
{
	if (NULL == s || s->str == NULL)
		return false;
	for (size_t i = 0; i < s->len; i++) {
		if (s->str[i] == c)
			return true;
	}
	return false;
}
