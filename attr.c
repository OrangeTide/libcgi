#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "attr.h"

struct attribute
{
	int type;
	char *value;
	int len; /* string length */
	/* int value; */
};
typedef struct attribute attr_t;

struct attribute_list
{
	int len;
	attr_t *data;
};

static struct name_list
{
	int len;
	char **data;
} name_list;

static int nameadd(const char *name)
{
	char **entry;
	int type;

	if(!name) abort();
	name_list.data=realloc(name_list.data,(name_list.len+1)*sizeof *name_list.data);
	type=name_list.len;
	entry=name_list.data+type;
	name_list.len++;
	*entry=strdup(name);	
	return type;
}

static int attrtype(const char *name)
{
	int i;
	for(i=0;i<name_list.len;i++) {
		if(!strcasecmp(name,name_list.data[i])) {
			return i;
		}
	}
	return -1; /* no name */
}

static attr_t *attrlookup(attrlist_t al, int type)
{
	int i;

	if(type<0) return NULL; /* not found */

	for(i=0;i<al->len;i++)
	{
		if(type==al->data[i].type) return al->data+i;
	}
	return NULL;
}

static attr_t *attradd(attrlist_t al, int type)
{
	attr_t *at;

	al->data=realloc(al->data,(al->len+1)*sizeof *al->data);
	at=al->data+al->len;
	al->len++;
	at->type=type;
	at->len=0;
	at->value=NULL;
	return at;	
}

static void attrdel(attrlist_t al, attr_t *at)
{
	int ofs;
	ofs=at-al->data;
	al->len--;
	al->data[ofs]=al->data[al->len];
}

static const char *attrname(int type)
{
	if(type<0 || type>=name_list.len) return NULL; /* no name */
	return name_list.data[type];
}

static attr_t *setup_access(attrlist_t al, const char *name) {
	attr_t *at;
	int type;

	type=attrtype(name);
	if(type==-1) { /* type not found add it to the global list */
		type=nameadd(name);	
	}
	at=attrlookup(al,type);
	if(!at) {
		/* create a new attribute if it doesn't exist */
		at=attradd(al,type);
	} 

	return at;
}

/*** EXPORTED STUFF ***/
void attrcatn(attrlist_t al, const char *name, const char *value, size_t len) {
	attr_t *at;
	size_t oldlen;
	char *newvalue;
	at=setup_access(al, name);	
	/* a null value would delete the attribute */
	if(value==NULL) {
		if(at) attrdel(al,at); /* delete if there is no value */
		return;
	}
	/* append the string */
	oldlen=at->len;
	newvalue=realloc(at->value, oldlen+len+1); 
	if(!newvalue) return;
	memcpy(newvalue+oldlen, value, len);
	at->value=newvalue;
}

void attrcat(attrlist_t al, const char *name, const char *value) {
	attrcatn(al, name, value, strlen(value));
}

/* set an attribute */
void attrsetn(attrlist_t al, const char *name, const char *value, size_t len)
{
	attr_t *at;
	at=setup_access(al, name);	
	/* a null value would delete the attribute */
	if(value==NULL) {
		if(at) attrdel(al,at); /* delete if there is no value */
		return;
	}
	/* discard the old attribute values */
	free(at->value);
	at->value=NULL;
	/* set the attribute */
	at->len=len;
	at->value=malloc(at->len+1);
	memcpy(at->value,value,at->len+1);
}

void attrset(attrlist_t al, const char *name, const char *value)
{
	attrsetn(al, name, value, strlen(value));
}

int attrvprintf(attrlist_t al, const char *name, const char *fmt, va_list ap)
{
	char buf[MAX_ATTR_LEN];
	int len;
	len=vsnprintf(buf, sizeof buf, fmt, ap);
	if(len>=0)
		attrset(al, name, buf);
	return len;
}

int attrprintf(attrlist_t al, const char *name, const char *fmt, ...)
{
	int len;
	va_list ap;
	va_start(ap, fmt);
	len=attrvprintf(al, name, fmt, ap);
	va_end(ap);
	return len;
}

/* get an attribute, return NULL if not found */
const char *attrget(attrlist_t al, const char *name)
{
	attr_t *at;
	int type;

	type=attrtype(name);
	at=attrlookup(al,type);
	return at?at->value:NULL;
}

int attrlist(attrlist_t al, const char **name, const char **value, int *counter)
{
	if((*counter)<al->len) {
		*name=attrname(al->data[*counter].type);
		*value=al->data[*counter].value;
		(*counter)++;
		return 1;
	} else {
		return 0;
	}
}

attrlist_t attrinit(void)
{
	attrlist_t al;	
	al=malloc(sizeof *al);
	al->len=0;
	al->data=NULL;
	return al;
}

void attrfree(attrlist_t al)
{
	int i;
	for(i=0;i<al->len;i++) {
		free(al->data[i].value);
	}
	free(al->data);
	free(al);
}

void namefree(void)
{
	free(name_list.data);
	name_list.data=NULL;
	name_list.len=0;
}

/*
void attrdump(attrlist_t al)
{
	int cnt;
	printf("CNT:%d\n",al->len);
	for(cnt=0;cnt<al->len;cnt++)
	{
		printf("%-20d|%40s\n",al->data[cnt].type,al->data[cnt].value);
	}
	printf("\n");
}
*/

