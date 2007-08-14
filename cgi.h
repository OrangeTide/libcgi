#ifndef CGI_H
#define CGI_H

#include <stdarg.h>
#include <stdio.h>
#include "attr.h"

/* biggest POST message we will process */
#define MAX_POST_LEN		1048576L

/* biggest cookie we will process */
#define MAX_COOKIE_LEN		8192

typedef struct cgi_t *cgi_t;

/* cgi.c */
cgi_t cgi_init(void);
int cgi_vprintf(cgi_t ht, const char *fmt, va_list ap);
int cgi_printf(cgi_t ht, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
void cgi_set_content_type(cgi_t ht, const char *content_type);
void cgi_set_cache_control(cgi_t ht, const char *cache_control);
void cgi_start_headers(cgi_t ht);
void cgi_setparam(cgi_t ht, const char *name, const char *val);
const char *cgi_param(cgi_t ht, const char *name);
attrlist_t cgi_attrlist(cgi_t ht);
void cgi_setenv(cgi_t ht, const char *name, const char *val);
const char *cgi_getenv(cgi_t ht, const char *name);
void cgi_free(cgi_t ht);

#endif /* CGI_H */
