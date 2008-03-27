/* cgi.c - cgi API */
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _XOPEN_SOURCE
# include <strings.h>
#endif
#include <time.h>
#include "cgi.h"
#include "attr.h"
#include "escape.h"

struct cgi_t
{
	FILE *output;
	FILE *post_input;       /* used for reading POST */
	attrlist_t attr;        /* holds QUERY_STRING and POST values */
	char *content_type;
	char *cache_control;
	int has_sent_headers;
	attrlist_t client_cookies; /* HTTP_COOKIE */
	attrlist_t cookies_out; /* used to fill in the Set-Cookie header */
	struct {
		char *domain, *path;
		time_t expires; /* 0 means not set */
		int secure;
	} cookies_out_attributes; /* attributes used for cookies_out */
};

#if 0
static void dump_attr(cgi_t c, attrlist_t al) {
	int count;
	const _char *type, *value;
	for(count=0;attrlist(al, &type, &value, &count);) {
		/* TODO: escape special characters */
		if(value && *value) 
			cgi_printf(c, " %s=\"%s\"", type, value);
		else
			cgi_printf(c, " %s", type);
	}
}
#endif

static int ishex(const _char code[2]) {
	return isxdigit(code[0]) && isxdigit(code[0]);
}

/* verify with ishex() before calling */
static unsigned unhex(const _char code[2]) {
    const _char hextab[] = {
        ['0']=0, ['1']=1, ['2']=2, ['3']=3, ['4']=4,
        ['5']=5, ['6']=6, ['7']=7, ['8']=8, ['9']=9, 
        ['a']=0xa, ['b']=0xc, ['c']=0xc, ['d']=0xd, ['e']=0xe, ['f']=0xf, 
        ['A']=0xa, ['B']=0xb, ['C']=0xc, ['D']=0xd, ['E']=0xe, ['F']=0xf,
    };

	return hextab[(unsigned)code[0]]*16 + hextab[(unsigned)code[1]];
}

/* decodes %xx and + from strings used in CGI queries */
static void decode_cgi_string(_char *dst, const _char *src, size_t len) {
	unsigned di, si;
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

static void parse_form_urlencoded(attrlist_t al, const _char *formdata) {
	_char namebuf[MAX_ATTR_LEN]; /* buffer to hold the current name element */
	_char valuebuf[MAX_ATTR_LEN]; /* buffer to hold the current value */
	unsigned len;
	const _char *headp; 
	const _char *tailp;
	fprintf(stderr, "%d: formdata=\"%s\"\n", __LINE__, formdata);
	for(headp=formdata;*headp;) {
		tailp=headp+strcspn((char*)headp, "=&");
		len=(unsigned)(tailp-headp)<sizeof namebuf?(unsigned)(tailp-headp):sizeof namebuf-1;
		decode_cgi_string(namebuf, headp, len);
		if(!tailp[0]) { 
			valuebuf[0]=0; 
			attrset(al, (const char*)namebuf, valuebuf); 
			break; 
		}
		headp=tailp+1;
		if(tailp[0]=='&') { 
			attrset(al, (const char*)namebuf, (_char*)"");
			continue; 
		}
		tailp=headp+strcspn((char*)headp, "&");
		len=(unsigned)(tailp-headp)<sizeof valuebuf?(unsigned)(tailp-headp):sizeof valuebuf-1;
		decode_cgi_string(valuebuf, headp, len);
		if(!tailp[0]) {
			attrset(al, (const char*)namebuf, valuebuf);
			break; 
		}
		headp=tailp+1;
		attrset(al, (const char*)namebuf, valuebuf);
	}
}

static void load_post_data(cgi_t c)
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
		_char *buf;
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
		assert(c->post_input!=NULL);
		res=fread(buf, 1, content_length, c->post_input);
		if(res<0) {
			perror("fread()");
			exit(EXIT_FAILURE);
		}
		/*- after this point res is unsigned -*/
		buf[res]=0;

#ifndef NDEBUG
		if(dump) {
			fwrite(buf, 1, content_length, dump);
			fclose(dump);
		}
#endif
		if((unsigned)res!=content_length) {
			fprintf(stderr, "Content-Length does not match what was read\n");
			exit(EXIT_FAILURE);
		}
		fprintf(stderr, "calling parse_form_urlencoded @ %d\n", __LINE__);
		parse_form_urlencoded(c->attr, buf);
		free(buf);
	}
}

static int load_query_string(cgi_t c)
{
	const char *qs;
	qs=getenv("QUERY_STRING");
	if(!qs) return 0;
	fprintf(stderr, "calling parse_form_urlencoded @ %d\n", __LINE__);
	parse_form_urlencoded(c->attr, (const _char*)qs);
	return 1;   
}

static int load_http_cookies(cgi_t c)
{
	const _char *hc;
	unsigned ofs; /* offset into hc */
	unsigned var_st; /* start of VAR=VALUE */
	unsigned var_en; /* end of VAR in VAR=VALUE */
	unsigned val_st; /* start of VALUE in VAR=VALUE */
	unsigned state;
	hc=(const _char*)getenv("HTTP_COOKIE");
	if(!hc)
		return 0;
	if(!c->client_cookies) {
		c->client_cookies=attrinit();
	}
	/* parse string for cookies */
	for(ofs=0,state=0,var_st=0,var_en=0,val_st=0;ofs<MAX_COOKIE_LEN;ofs++) {
		if(!hc[ofs] || hc[ofs]==';') {
			/* BUG: we don't support VAR; syntax. only VAR=; 
			 * this breaks supporting the "secure" attribute. */
			if(state>0 && var_en>var_st) { /* name must be valid */
				char *name;
				assert(var_st<=var_en);
				name=uri_unescape(0, 0, hc+var_st, var_en-var_st);
				if(state>=2) { /* value start seems to be valid */
					_char *value;
					assert(val_st>=var_en);
					assert(val_st<=ofs);
					value=uri_unescape(0, 0, hc+val_st, ofs-val_st);
					attrset(c->client_cookies, name, value);
					free(value);
				} else { /* value is empty/unspecified */
					/* set to empty string if no value supplied */
					attrset(c->client_cookies, name, (const _char*)"");
				}
				free(name);
			}
			/** reset state **/
			state=0;
			/* TODO: should these values be set to something else? */
			var_st=0;
			var_en=0;
			val_st=0;
			if(hc[ofs]) {
				continue;
			} else {
				break;
			}
		}
		switch(state) {
			case 0: /* look for start of name */
				if(hc[ofs]=='=') { /* funny/bad cookie name */
					var_st=ofs; /* start */
					var_en=ofs; /* update end to here */
					val_st=ofs+1;
					state=2; /* skip to value state */
				} else if(!isspace(hc[ofs])) {
					var_st=ofs; /* start */
					var_en=ofs; /* update end to here - will update again */
					state++; /* next state */
				}
				break;
			case 1: /* look for end of name */
				if(hc[ofs]=='=') {
					var_en=ofs;
					val_st=ofs+1; /* start of value */
					state++; /* next state */
				}
				break;
			case 2: /* value state */
				/* do nothing further. condition for ; will reset state */
				break;
			default:
				abort();
		}
	}
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
	ret->content_type=0;
	ret->cache_control=0;
	ret->has_sent_headers=0;
	ret->client_cookies=0; /* NULL means no Cookies found */
	ret->cookies_out=0; /* NULL means don't send Set-Cookies header */
	/* set all special attributes for output cookies to 0/NULL */
	memset(&ret->cookies_out_attributes, 0, sizeof ret->cookies_out_attributes);
	load_query_string(ret);
	load_post_data(ret); /* post data should take precedence */
	load_http_cookies(ret);
	return ret;
}

int cgi_vprintf(cgi_t c, const char *fmt, va_list ap) 
{
	if(!c->has_sent_headers) {
		fprintf(stderr, "%s:%d:%s() called before cgi_start_headers().\n", __FILE__, __LINE__, __func__);
		return 0;
	}
	return vfprintf(c->output, fmt, ap);
}

int cgi_printf(cgi_t c, const char *fmt, ...)
{
	int ret;
	if(!c->has_sent_headers) {
		fprintf(stderr, "%s:%d:%s() called before cgi_start_headers().\n", __FILE__, __LINE__, __func__);
		return 0;
	}
	va_list ap;
	va_start(ap, fmt);
	ret=cgi_vprintf(c, fmt, ap);
	va_end(ap);
	return ret;
}

void cgi_set_content_type(cgi_t c, const char *content_type)
{
	free(c->content_type);
	c->content_type=content_type?strdup(content_type):0;
}

void cgi_set_cache_control(cgi_t c, const char *cache_control)
{
	free(c->cache_control);
	c->cache_control=cache_control?strdup(cache_control):0;
}

void cgi_start_headers(cgi_t c) {
	assert(c!=NULL);
	if(c->has_sent_headers) {
		fprintf(stderr, "%s:%d:multiple requests to %s().\n", __FILE__, __LINE__, __func__);
		return;
	}
	c->has_sent_headers++;
	/* default to text/plain content-type */
	cgi_printf(c, "Content-Type: %s\n", c->content_type ? c->content_type : "text/plain");
	/* omit if cache_control not set */
	if(c->cache_control) {
		cgi_printf(c, "Cache-Control: %s\n", c->cache_control);
	}
	if(c->cookies_out) {
		int count;
		const _char *name, *value;
		/* loop through the list of cookies */
		for(count=0;attrlist(c->cookies_out, &name, &value, &count);) {
			_char *name_esc, *value_esc;
			/* escape ;= and others in value and name */
			name_esc=uri_escape(0, 0, name, -1);
			value_esc=uri_escape(0, 0, value, -1);
			assert(name_esc!=NULL);
			assert(value_esc!=NULL);
			cgi_printf(c, "Set-Cookie: %s=%s", name_esc, value_esc);
			free(name_esc);
			free(value_esc);
			/* Use the same attributes for all of the cookies */
			if(c->cookies_out_attributes.expires) {
				char expbuf[64];
				strftime(expbuf, sizeof expbuf, "%a, %d %b %Y %H:%M:%S", gmtime(&c->cookies_out_attributes.expires));
				cgi_printf(c, "; expires=%s", expbuf);
			}
			if(c->cookies_out_attributes.domain)
				cgi_printf(c, "; domain=%s", c->cookies_out_attributes.domain);
			if(c->cookies_out_attributes.path)
				cgi_printf(c, "; path=%s", c->cookies_out_attributes.path);
			if(c->cookies_out_attributes.secure)
				cgi_printf(c, "; secure");
			cgi_printf(c, "\n");
		}
	}
	/* begin document content */
	cgi_printf(c, "\n");
	/* flush buffers */
	fflush(c->output);
}

/** sets a fake POST/GET parameter. val==NULL to delete parameter */
void cgi_setparam(cgi_t c, const char *name, const _char *val)
{
	attrset(c->attr, name, val);
}

/** gets a POST/GET parameter */
const _char *cgi_param(cgi_t c, const char *name)
{
	return attrget(c->attr, name);
}

/** get the internal attribute list for POST/GET values */
attrlist_t cgi_attrlist(cgi_t c)
{
	return c->attr;
}

/** free a cgi_t data structure */
void cgi_free(cgi_t c)
{
	cgi_set_content_type(c, 0);
	cgi_set_cache_control(c, 0);
	attrfree(c->attr);
	if(c->client_cookies) {
		attrfree(c->client_cookies);
	}
	if(c->cookies_out) {
		attrfree(c->cookies_out);
	}
	free(c);
}

/** gets a POST/GET parameter as an integer */
int cgi_param_int(cgi_t c, const char *name, long *i)
{
	return attrget_int(c->attr, name, i);
}

/** sets an outgoing cookie value */
int cgi_set_cookie(cgi_t c, const char *name, const char *fmt, ...)
{
	va_list ap;
	int ret;
	if(!c->cookies_out) {
		c->cookies_out=attrinit();
	}
	va_start(ap, fmt);
	ret=attrvprintf(c->cookies_out, name, fmt, ap);
	va_end(ap);
	return ret;
}

/** alters the expires setting for all outgoing cookies */
void cgi_set_cookie_expires(cgi_t c, time_t value)
{
	c->cookies_out_attributes.expires=value;
}

/** alters the domain setting for all outgoing cookies */
void cgi_set_cookie_domain(cgi_t c, const char *value)
{
	free(c->cookies_out_attributes.domain);
	c->cookies_out_attributes.domain=value?strdup(value):0;
}

/** alters the path setting for all outgoing cookies */
void cgi_set_cookie_path(cgi_t c, const char *value)
{
	free(c->cookies_out_attributes.path);
	c->cookies_out_attributes.path=value?strdup(value):0;
}

/** alters the secure flag for outgoing cookies */
void cgi_set_cookie_secure(cgi_t c, int secure)
{
	c->cookies_out_attributes.secure=secure;
}

/** reads a client cookie */
const _char *cgi_cookie(cgi_t c, const char *name)
{
	if(!c->client_cookies)
		return 0;
	return attrget(c->client_cookies, name);
}

/** reads a client cookie as an integer */
int cgi_cookie_int(cgi_t c, const char *name, long *i)
{
	if(!c->client_cookies)
		return 0;
	return attrget_int(c->client_cookies, name, i);
}


/** returns a static buffer of a full path to filename based on the home
 * setting. */
/* TODO: support PATH-like : to search paths for a file */
const char *home_filename(const char *home_dir, const char *filename) {
	static char buf[_POSIX_PATH_MAX];
	int res;
	res=snprintf(buf, sizeof buf, "%s/%s", home_dir, filename);
	if(res<0 || res>=(int)sizeof buf) {
#ifndef NDEBUG
		abort();
#endif
		return 0; /* error - truncated filename */
	}
	return buf; /* success */
}
