#ifndef CGI_H
#define CGI_H

#include <stdio.h>

typedef struct html_t *html_t;

/* cgi.c */
html_t htmlcreate ( FILE *output , const char *title );
void htmlfree ( html_t ht );
void htmlbegin ( html_t ht );
void htmlparagraph ( html_t ht , const char *fmt , ...);
void htmlend ( html_t ht );
void htmllistbegin ( html_t ht );
void htmllist ( html_t ht , const char *fmt , ...);
void htmllistend ( html_t ht );
void htmlcontent(html_t ht, const char *content_type);
void htmlhyperlink(html_t ht, const char *href, const char  *label, ...);
void htmlheader(html_t ht, int level, const char *fmt, ...);
const char *htmlget(html_t ht, const char *name);

#endif /* CGI_H */
