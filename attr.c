#include <stdlib.h>
#include <string.h>
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
		if(!strcmp(name,name_list.data[i])) {
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
	at->value=NO_ATTR;
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

/*** EXPORTED STUFF ***/

/* set an attribute */
void attrset(attrlist_t al,const char *name, const char *value)
{
	attr_t *at;
	int type;
	
	type=attrtype(name);
	if(type==-1) { /* type not found add it to the global list */
		type=nameadd(name);	
	}
	at=attrlookup(al,type);
	if(!at && value!=NO_ATTR) {
		/* create a new attribute if it doesn't exist */
		at=attradd(al,type);
	} 

	if(value==NO_ATTR) {
		if(at) attrdel(al,at); /* delete if there is no value */
		return;
	}
	/* discard the old attribute values */
	free(at->value);
	at->value=NO_ATTR;
	/* set the attribute */
	at->len=strlen(value);
	at->value=malloc(at->len+1);
	memcpy(at->value,value,at->len+1);
}

/* get an attribute, return NO_ATTR if not found */
const char *attrget(attrlist_t al, const char *name)
{
	attr_t *at;
	int type;

	type=attrtype(name);
	at=attrlookup(al,type);
	return at?at->value:NO_ATTR;
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

