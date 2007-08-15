#ifndef ATTR_H
#define ATTR_H
#include <stdlib.h>
#include <stdarg.h>

#if 1
/* use to force most strings to be unsigned */
typedef unsigned char _char; 
#else
/* keep strings as ordinary char */
typedef char _char;
#endif

#define MAX_ATTR_LEN	1024	/* an arbitrary limit */
typedef struct attribute_list *attrlist_t;

/* attr.c */
void attrcatn(attrlist_t al, const char *name, const _char *value, size_t len);
void attrcat(attrlist_t al, const char *name, const _char *value);
void attrsetn(attrlist_t al, const char *name, const _char *value, size_t len);
void attrset(attrlist_t al, const char *name, const _char *value);
void attrset_safe(attrlist_t al, const char *name, const _char *value);
int attrprintf(attrlist_t al, const char *name, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));
int attrvprintf(attrlist_t al, const char *name, const char *fmt, va_list ap);
const _char *attrget(attrlist_t al, const char *name);
int attrlist(attrlist_t al, const _char **type, const _char **value, int *counter);
attrlist_t attrinit(void);
void attrfree(attrlist_t al);
void namefree(void);
int attrget_int(attrlist_t al, const char *name, long *i);

#ifndef NDEBUF
void attrdump(FILE *out, attrlist_t al);
#endif
#endif /* ATTR_H */
