#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>

static int 
parse_url(const char *url, char *host,
        size_t host_len, char *path,
        size_t path_len, int *port)
{
    /* TODO: give back a struct */

    /* check if it's http(s) */
    if (!(strncmp(url, "https://", 8))) {
      url += 8;
      *port = HTTPS_PORT;
    } else if (!(strncmp(url, "http://", 7))) {
      url += 7;
      *port = HTTP_PORT;
    } else {
      return -1; /* wrong protocol */
    }
    const char *slash = strchr(url, '/');
    const char *colon = strchr(url, ':');

    /* check for custom port */
    if (colon && (!slash || colon < slash)) {
      *port = atoi(colon + 1);
      strncpy(host, url, slash - url);
      host[colon - url] = '\0';
    } else {
      if (slash)
        strncpy(host, url, slash - url);
      else
        strncpy(host, url, host_len - 1);
      
      host[host_len] = '\0';
    }

    /* is there any path? */
    if (slash)
      strncpy(path, slash, path_len - 1);
    else
      strcpy(path, "/");

    return 0;
}

int
http_request(const char *url,
        const char *method,
        const char *headers,
        const char *body,
        char **response,
        size_t *resp_len)
{
  int ret;
  char host[256], path[1024];
  int port;

  if (parse_url(url, host, sizeof(host), path, sizeof(path), &port) != 0)
    return -1; /* error! */

  /* init mbedtls */
  mbedtls_net_context net;
  mbedtls_ssl_context ssl;
  mbedtls_ssl_config conf;
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;

  mbedtls_net_init(&net);
  mbedtls_ssl_init(&ssl);
  mbedtls_ssl_config_init(&conf);
  mbedtls_entropy_init(&entropy);
  mbedtls_ctr_drbg_init(&ctr_drbg);

  if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg,
                  mbedtls_entropy_func, &entropy, NULL, 0)) != 0)
      goto cleanup; /* i'm not sure about that */

  if ((ret = mbedtls_net_connect(&net, host,
                  (port == HTTPS_PORT) ? "443" : "80",
                  MBEDTLS_NET_PROTO_TCP)) != 0)
      goto cleanup;

  if (port == HTTPS_PORT) {
    if ((ret = mbedtls_ssl_config_defaults(&conf, 
                    MBEDTLS_SSL_IS_CLIENT,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
      goto cleanup;

    /* TODO: write cert verefication code */
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_NONE); /* for test */
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
        goto cleanup;

    if ((ret = mbedtls_ssl_set_hostname(&ssl, host)) != 0)
        goto cleanup;

    mbedtls_ssl_set_bio(&ssl, &net, mbedtls_net_send, mbedtls_net_recv, NULL);

    while ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
      if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
          goto cleanup;
    }
  }

  /* create request */
  char req[2048];
  snprintf(req, sizeof(req), 
          "%s %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n%s\r\n%s",
          method, path, host, headers ? headers : "",
          body ? body : "");

  /* sending, yay! */
  if (port == HTTPS_PORT)
    ret = mbedtls_ssl_write(&ssl, (unsigned char *)req, strlen(req));
  else
    ret = mbedtls_net_send(&net, (unsigned char *)req, strlen(req));

  if (ret <= 0)
    goto cleanup;

  /* reading response */
  size_t cap = 8192;
  *response = malloc(cap);
  *resp_len = 0;

  while (1) {
    unsigned char buf[1024];
    int len = (port == HTTPS_PORT)
                  ? mbedtls_ssl_read(&ssl, buf, sizeof(buf))
                  : mbedtls_net_recv(&net, buf, sizeof(buf));
    if (len <= 0)
      break;
    if (*resp_len + len > cap) {
      cap *= 2;
      *response = realloc(*response, cap);
    }
    memcpy(*response + *resp_len, buf, len);
    *resp_len += len;
  }

  ret = 0;

cleanup:
  mbedtls_net_init(&net);
  mbedtls_ssl_init(&ssl);
  mbedtls_ssl_config_init(&conf);
  mbedtls_entropy_init(&entropy);
  mbedtls_ctr_drbg_init(&ctr_drbg);

  return ret;
}
