#ifndef ATTR_H
#define ATTR_H
#include <stdlib.h>
#define NO_ATTR			(NULL)	/* no attribute */
#define MAX_ATTR_LEN	1024	/* an arbitrary limit */
typedef struct attribute_list *attrlist_t;

/* attr.c */
void attrset ( attrlist_t al , const char *name , const char *value );
const char *attrget ( attrlist_t al , const char *name );
int attrlist ( attrlist_t al , const char **type , const char **value , int *counter );
attrlist_t attrinit();
void attrfree(attrlist_t al);
void namefree(void);

#endif /* ATTR_H */
