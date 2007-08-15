#include <assert.h>
#include <stdlib.h>
#include <string.h>
#ifdef _XOPEN_SOURCE
# include <strings.h>
#endif
#include <stdio.h>
#include <stdarg.h>
#include "attr.h"

struct attribute
{
	int type;
	_char *value;
	unsigned len; /* string length */
	/* int value; */
};
typedef struct attribute attr_t;

struct attribute_list
{
	unsigned len;
	attr_t *data;
};

static struct name_list
{
	unsigned len;
	_char **data;
	/* TODO: keep a reference count */
} name_list;

static int nameadd(const _char *name)
{
	_char **entry;
	int type;

	if(!name) abort();
	name_list.data=realloc(name_list.data, (name_list.len+1)*sizeof *name_list.data);
	type=name_list.len;
	entry=name_list.data+type;
	name_list.len++;
	*entry=(_char*)strdup((const char*)name);	
	return type;
}

/** gets the index in the name list of a given attribute name **/
static int attrtype(const _char *name)
{
	unsigned i;
	for(i=0;i<name_list.len;i++) {
		if(!strcasecmp((const char*)name, (const char*)name_list.data[i])) {
			return i;
		}
	}
	return -1; /* no name */
}

static attr_t *attrlookup(attrlist_t al, int type)
{
	unsigned i;

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

	al->data=realloc(al->data, (al->len+1)*sizeof *al->data);
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

static const _char *attrname(int type)
{
	if(type<0 || (unsigned)type>=name_list.len) return NULL; /* no name */
	return name_list.data[type];
}

static attr_t *setup_access(attrlist_t al, const _char *name) {
	attr_t *at;
	int type;

	type=attrtype(name);
	if(type==-1) { /* type not found add it to the global list */
		type=nameadd(name);	
	}
	at=attrlookup(al, type);
	if(!at) {
		/* create a new attribute if it doesn't exist */
		at=attradd(al, type);
	} 

	return at;
}

/*** EXPORTED STUFF ***/
void attrcatn(attrlist_t al, const _char *name, const _char *value, size_t len) {
	attr_t *at;
	size_t oldlen;
	_char *newvalue;
	at=setup_access(al, name);	
	/* a null value would delete the attribute */
	if(value==NULL) {
		if(at) attrdel(al, at); /* delete if there is no value */
		return;
	}
	/* append the string */
	oldlen=at->len;
	newvalue=realloc(at->value, oldlen+len+1); 
	if(!newvalue) return;
	memcpy(newvalue+oldlen, value, len);
	at->value=newvalue;
}

void attrcat(attrlist_t al, const _char *name, const _char *value) {
	attrcatn(al, name, value, value ? strlen((const char*)value) : 0);
}

static unsigned escape_len(const _char *s, size_t len) {
	unsigned ret;
	for(ret=0;len && *s;s++) {
		switch(*s) {
			case '<': ret+=4; len-=4; break; /* &lt; */
			case '>': ret+=4; len-=4; break; /* &gt; */
			case '&': ret+=5; len-=5; break; /* &amp; */
			default:
				ret++;
				len--;
		}
	}
	return ret;
}

static unsigned safe_append(_char *dest, size_t len, const _char *str) {
	unsigned len2;
	len2=strlen((const char*)str);
	if(len2>len) {
		return 0; /* refuse to append */
	}
	memcpy(dest, str, len2+1);
	return len2;
}

static void html_escape(_char *dest, size_t len, const _char *s) {
	unsigned i;

	for(i=0;i<len && *s;s++) {
		switch(*s) {
			case '<':
				i+=safe_append(dest+i,len-i, (_char*)"&lt;");	
				break;
			case '>':
				i+=safe_append(dest+i,len-i, (_char*)"&gt;");	
				break;
			case '&':
				i+=safe_append(dest+i,len-i, (_char*)"&amp;");	
				break;
			default:
				dest[i++]=*s;
		}
	}
	dest[i]=0;
}

/* set an attribute */
static void attrsetn_internal(attrlist_t al, const _char *name, int safe_fl, const _char *value, size_t len)
{
	attr_t *at;
	at=setup_access(al, name);	
	/* a null value would delete the attribute */
	if(value==NULL) {
		if(at) attrdel(al, at); /* delete if there is no value */
		return;
	}
	/* discard the old attribute values */
	free(at->value);
	at->value=NULL;
	/* set the attribute */
	if(safe_fl) {
		at->len=escape_len(value, len);
		at->value=malloc(at->len+1);
		html_escape(at->value, at->len, value);
	} else {
		at->len=len;
		at->value=malloc(len+1);
		memcpy(at->value, value, at->len+1);
	}
}

#ifndef NDEBUF
void attrdump(FILE *out, attrlist_t al)
{
	unsigned cnt;
	fprintf(out, "CNT:%u\n", al->len);
	for(cnt=0;cnt<al->len;cnt++)
	{
		fprintf(out, "%-20d|%40s\n", al->data[cnt].type, al->data[cnt].value);
	}
	fprintf(out, "\n");
}
#endif

void attrsetn(attrlist_t al, const _char *name, const _char *value, size_t len) {
	attrsetn_internal(al, name, 0, value, len);
}

void attrset(attrlist_t al, const _char *name, const _char *value) {
	attrsetn_internal(al, name, 0, value, value ? strlen((const char*)value) : 0);
}

void attrset_safe(attrlist_t al, const _char *name, const _char *value) {
	attrsetn_internal(al, name, 1, value, value ? strlen((const char*)value) : 0);
}

int attrvprintf(attrlist_t al, const _char *name, const char *fmt, va_list ap)
{
	_char buf[MAX_ATTR_LEN];
	int len;
	len=vsnprintf((char*)buf, sizeof buf, fmt, ap);
	if(len>=0)
		attrset(al, name, buf);
	return len;
}

int attrprintf(attrlist_t al, const _char *name, const char *fmt, ...)
{
	int len;
	va_list ap;
	va_start(ap, fmt);
	len=attrvprintf(al, name, fmt, ap);
	va_end(ap);
	return len;
}

/* get an attribute, return NULL if not found */
const _char *attrget(attrlist_t al, const _char *name)
{
	attr_t *at;
	int type;

	type=attrtype(name);
	at=attrlookup(al, type);
	return at?at->value:NULL;
}

int attrlist(attrlist_t al, const _char **name, const _char **value, int *counter)
{
	if((unsigned)(*counter)<al->len) {
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
	unsigned i;
	for(i=0;i<al->len;i++) {
		free(al->data[i].value);
	}
	free(al->data);
	free(al);
}

void namefree(void)
{
	unsigned i;
	for(i=0;i<name_list.len;i++) {
		free(name_list.data[i]);
	}
	free(name_list.data);
	name_list.data=NULL;
	name_list.len=0;
}

/* gets an attribute as a numeric value */
int attrget_int(attrlist_t al, const _char *name, long *i) {
	long ret;
	const char *tmp;
	char *endptr;

	assert(i!=NULL);
	assert(al!=NULL);
	assert(name!=NULL);
	tmp=(const char*)attrget(al, name);
	if(!tmp) {
		return 0; /* failure */
	}
	ret=strtoul(tmp, &endptr, 10);
	assert(endptr!=NULL);
	if(endptr==tmp || *endptr) {
		return 0; /* failure */
	}
	*i=ret;
	return 1; /* success */
}

