#ifndef ATTR_H
#define ATTR_H
#include <stdlib.h>
#include <stdarg.h>
#define MAX_ATTR_LEN	1024	/* an arbitrary limit */
typedef struct attribute_list *attrlist_t;

/* attr.c */
void attrcatn(attrlist_t al, const char *name, const char *value, size_t len);
void attrcat(attrlist_t al, const char *name, const char *value);
void attrsetn(attrlist_t al, const char *name, const char *value, size_t len);
void attrset(attrlist_t al, const char *name, const char *value);
void attrset_safe(attrlist_t al, const char *name, const char *value);
int attrprintf(attrlist_t al, const char *name, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));
int attrvprintf(attrlist_t al, const char *name, const char *fmt, va_list ap);
const char *attrget ( attrlist_t al , const char *name );
int attrlist ( attrlist_t al , const char **type , const char **value , int *counter );
attrlist_t attrinit(void);
void attrfree(attrlist_t al);
void namefree(void);

#endif /* ATTR_H */
