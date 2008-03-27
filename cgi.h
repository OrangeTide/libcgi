#ifndef CGI_H
#define CGI_H

#include <stdarg.h>
#include <stdio.h>
#include <time.h>
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
void cgi_setparam(cgi_t ht, const char *name, const _char *val);
const _char *cgi_param(cgi_t ht, const char *name);
attrlist_t cgi_attrlist(cgi_t ht);
void cgi_free(cgi_t ht);
int cgi_param_int(cgi_t c, const char *name, long *i);
int cgi_set_cookie(cgi_t c, const char *name, const char *fmt, ...) __attribute__( (format (printf, 3, 4)));
void cgi_set_cookie_expires(cgi_t c, time_t value);
void cgi_set_cookie_domain(cgi_t c, const char *value);
void cgi_set_cookie_path(cgi_t c, const char *value);
void cgi_set_cookie_secure(cgi_t c, int secure);
const _char *cgi_cookie(cgi_t c, const char *name);
int cgi_cookie_int(cgi_t c, const char *name, long *i);
const char *home_filename(const char *home_dir, const char *filename);
#endif /* CGI_H */
