#ifndef LIBMAU_HTTP_H
#define LIBMAU_HTTP_H

#include <stddef.h>

#define HTTP_PORT 80
#define HTTPS_PORT 443

/* returns 0 in case of success,
 * 1 in case of failure
 * url: "https://exapmle.com/path"
 * method: "GET", "POST", etc.
 * headers: request headers or NULL
 * body: request body or NULL
 * response: it's response
 * resp_len: lenght of response
 * */

int
http_request(const char *url,
        const char *method,
        const char *headers,
        const char *body,
        char **response,
        size_t *resp_len);

#endif
