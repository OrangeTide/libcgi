#ifndef CGI_H
#define CGI_H

#include <stdarg.h>
#include <stdio.h>
#include "attr.h"

/* biggest POST message we will process */
#define MAX_POST_LEN		1048576L

typedef struct html_t *html_t;

#define htmllist(ht, ...) htmltag((ht), "LI", 0, __VA_ARGS__)
#define htmldefterm(ht, ...) htmltag((ht), "DT", 0, __VA_ARGS__)
#define htmldefdescription(ht, ...) htmltag((ht), "DD", 0, __VA_ARGS__)
#define htmlparagraph(ht, ...) htmltag((ht), "P", 0, __VA_ARGS__)
#define htmllistbegin(ht) htmltagbegin((ht), "UL", 0)
#define htmllistend(ht) htmltagend((ht), "UL")
#define htmldeflistbegin(ht) htmltagbegin((ht), "DL", 0)
#define htmldeflistend(ht) htmltagend((ht), "DL")

html_t htmlcreate ( FILE *output , const char *title );
void htmlfree ( html_t ht );
void htmlbegin ( html_t ht );
void htmlend ( html_t ht );
void htmltagbegin ( html_t ht , const char *tag, attrlist_t attr);
void htmltagend ( html_t ht , const char *tag );
void htmltag ( html_t ht , const char *tag, attrlist_t attr, const char *fmt , ...);
void htmlcontent(html_t ht, const char *content_type);
char *htmlmakelink(const char *href, const char  *label, ...);
void htmlhyperlink(html_t ht, const char *href, const char  *label, ...);
void htmlheader(html_t ht, int level, const char *fmt, ...);
const char *htmlget(html_t ht, const char *name);
void htmlset(html_t ht, const char *name, const char *val);
attrlist_t htmlattrlist ( html_t ht );
int htmlvprintf(html_t ht, const char *fmt, va_list ap);
int htmlprintf(html_t ht, const char *fmt, ...);

#endif /* CGI_H */
