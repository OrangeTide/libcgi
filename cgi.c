#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _XOPEN_SOURCE
# include <strings.h>
#endif
#ifndef NDEBUG
#include <time.h>
#endif
#include "cgi.h"
#include "attr.h"

struct cgi_t
{
	FILE *output;
	FILE *post_input;       /* used for reading POST */
	char *title;
	attrlist_t attr;        /* holds QUERY_STRING and POST values */
	attrlist_t cgienv;		/* holds special parameters for page rendering */
};

static void dump_attr(cgi_t ht, attrlist_t al) {
	int count;
	const char *type, *value;
	for(count=0;attrlist(al, &type, &value, &count);) {
		/* TODO: escape special characters */
		if(value && *value) 
			cgi_printf(ht, " %s=\"%s\"", type, value);
		else
			cgi_printf(ht, " %s", type);
	}
}

static int ishex(const char code[2]) {
	return isxdigit(code[0]) && isxdigit(code[0]);
}

/* verify with ishex() */
static unsigned unhex(const char code[2]) {
    const char hextab[] = {
        ['0']=0, ['1']=1, ['2']=2, ['3']=3, ['4']=4,
        ['5']=5, ['6']=6, ['7']=7, ['8']=8, ['9']=9, 
        ['a']=0xa, ['b']=0xc, ['c']=0xc, ['d']=0xd, ['e']=0xe, ['f']=0xf, 
        ['A']=0xa, ['B']=0xb, ['C']=0xc, ['D']=0xd, ['E']=0xe, ['F']=0xf,
    };

	return hextab[(unsigned)code[0]]*16 + hextab[(unsigned)code[1]];
}

static void escstr(char *dst, const char *src, size_t len) {
	unsigned di,si;
	for(di=0,si=0;si<len;) {
		if(src[si]=='%' && si+2<len) {
			if(ishex(src+si+1)) {
				dst[di++]=unhex(src+si+1);
				si+=3;
			} else {
				dst[di++]=src[si++];
			}
		} else if(src[si]=='+') {
			dst[di++]=' ';
			si++;
		} else {
			dst[di++]=src[si++];
		}
	}
	dst[di]=0;
}

static void parse_form_urlencoded(attrlist_t al, const char *formdata) {
	char namebuf[MAX_ATTR_LEN]; /* buffer to hold the current name element */
	char valuebuf[MAX_ATTR_LEN]; /* buffer to hold the current value */
	int len;
	const char *headp; 
	const char *tailp;
	fprintf(stderr, "%d: formdata=\"%s\"\n", __LINE__, formdata);
	for(headp=formdata;*headp;) {
		tailp=headp+strcspn(headp,"=&");
		len=tailp-headp<sizeof namebuf?tailp-headp:sizeof namebuf-1;
		escstr(namebuf,headp,len);
		if(!tailp[0]) { valuebuf[0]=0; attrset(al,namebuf,valuebuf); break; }
		headp=tailp+1;
		if(tailp[0]=='&') { 
			attrset(al,namebuf,"");
			continue; 
		}
		tailp=headp+strcspn(headp,"&");
		len=tailp-headp<sizeof valuebuf?tailp-headp:sizeof valuebuf-1;
		escstr(valuebuf,headp,len);
		if(!tailp[0]) {
			attrset(al,namebuf,valuebuf);
			break; 
		}
		headp=tailp+1;
		attrset(al,namebuf,valuebuf);
	}
}

static void load_post_data(cgi_t ht)
{
	const char *rm;
	const char *cl;
	unsigned content_length;
	fprintf(stderr, "enter %s\n", __func__);
	rm=getenv("REQUEST_METHOD");
	fprintf(stderr, "%s:%d: REQUEST_METHOD=\"%s\"\n", __FILE__, __LINE__, rm);
	/* only do the POST stuff if we're doing POST method */
	if(!rm || strcasecmp(rm, "POST")) 
		return;
	cl=getenv("CONTENT_LENGTH");
	if(!cl)
		return;
	content_length=strtol(cl, 0, 0);
#ifndef NDEBUG
	fprintf(stderr, "%s:%d: content_length=%u\n", __FILE__, __LINE__, content_length);
#endif
	if(content_length > 0 && content_length < MAX_POST_LEN)  {
		char *buf;
		int res;
#ifndef NDEBUG
		FILE *dump;
		dump=fopen("dump", "w"); /* output to a dump file */
		if(!dump) {
			fprintf(stderr, "%s:%d:WARN:file 'dump' not writable, will not store post data.\n", __FILE__, __LINE__);
		}
#endif
		buf=malloc(content_length+1);
		if(!buf) {
			fprintf(stderr, "%s:%d:ERROR:malloc(%u) failed!\n", __FILE__, __LINE__, content_length);
			return;
		}
		assert(ht->post_input!=NULL);
		res=fread(buf, 1, content_length, ht->post_input);
		if(res<0) {
			perror("fread()");
			exit(EXIT_FAILURE);
		}
		buf[res]=0;

#ifndef NDEBUG
		if(dump) {
			fwrite(buf, 1, content_length, dump);
			fclose(dump);
		}
#endif
		if(res!=content_length) {
			fprintf(stderr, "Content-Length does not match what was read\n");
			exit(EXIT_FAILURE);
		}
		fprintf(stderr, "calling parse_form_urlencoded @ %d\n", __LINE__);
		parse_form_urlencoded(ht->attr, buf);
		free(buf);
	}
}

static int load_query_string(cgi_t ht)
{
	const char *qs;
	qs=getenv("QUERY_STRING");
	if(!qs) return 0;
	fprintf(stderr, "calling parse_form_urlencoded @ %d\n", __LINE__);
	parse_form_urlencoded(ht->attr, qs);
	return 1;   
}

/* TODO: implement flags to disable POST and/or GET methods */
cgi_t cgi_init(void) 
{
	cgi_t ret;

#ifndef NDEBUG
	freopen("log", "a", stderr);
	{
		time_t t;
		time(&t);
		fprintf(stderr, "-- %s", ctime(&t));
	}
#endif

	ret=malloc(sizeof *ret);
	ret->output=stdout;
	ret->post_input=stdin;
	ret->attr=attrinit();
	ret->cgienv=attrinit();
	load_query_string(ret);
	load_post_data(ret); /* post data should take precedence */
	return ret;
}

int cgi_vprintf(cgi_t ht, const char *fmt, va_list ap) 
{
	return vfprintf(ht->output, fmt, ap);
}

int cgi_printf(cgi_t ht, const char *fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret=cgi_vprintf(ht, fmt, ap);
	va_end(ap);
	return ret;
}

void cgi_content(cgi_t ht, const char *content_type)
{
	cgi_printf(ht,"Content-Type: %s\n",content_type);
	cgi_printf(ht,"Cache-Control: no-cache\n");
	cgi_printf(ht, "\n");
}

void cgi_setparam(cgi_t ht, const char *name, const char *val)
{
	attrset(ht->attr, name, val);
}

const char *cgi_param(cgi_t ht, const char *name)
{
	return attrget(ht->attr,name);
}

attrlist_t cgi_attrlist(cgi_t ht)
{
	return ht->attr;
}

void cgi_setenv(cgi_t ht, const char *name, const char *val)
{
	attrset(ht->cgienv, name, val);
}

const char *cgi_getenv(cgi_t ht, const char *name)
{
	return attrget(ht->cgienv,name);
}

