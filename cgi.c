#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef NDEBUG
#include <time.h>
#endif
#include "cgi.h"
#include "attr.h"

struct html_t
{
	FILE *output;
	FILE *post_input;       /* used for reading POST */
	char *title;
	attrlist_t attr;        /* holds QUERY_STRING and POST values */
	attrlist_t cgienv;		/* holds special parameters for page rendering */
};

static void dump_attr(html_t ht, attrlist_t al) {
	int count;
	const char *type, *value;
	for(count=0;attrlist(al, &type, &value, &count);) {
		/* TODO: escape special characters */
		htmlprintf(ht, " %s=\"%s\"", type, value);
	}
}

void htmlset(html_t ht, const char *name, const char *val)
{
	attrset(ht->attr, name, val);
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
		if(len) memcpy(namebuf,headp,len);
		namebuf[len]=0;
		if(!tailp[0]) { valuebuf[0]=0; attrset(al,namebuf,valuebuf); break; }
		headp=tailp+1;
		if(tailp[0]=='&') { valuebuf[0]=0; attrset(al,namebuf,valuebuf); continue; }
		tailp=headp+strcspn(headp,"&");
		len=tailp-headp<sizeof valuebuf?tailp-headp:sizeof valuebuf-1;
		if(len) memcpy(valuebuf,headp,len);
		valuebuf[len]=0;
		if(!tailp[0]) { attrset(al,namebuf,valuebuf); break; }
		headp=tailp+1;
		attrset(al,namebuf,valuebuf);
	}
}

static void load_post_data(html_t ht)
{
	const char *rm;
	const char *cl;
	long content_length;
	fprintf(stderr, "enter %s\n", __func__);
	rm=getenv("REQUEST_METHOD");
	/* only do the POST stuff if we're doing POST method */
	if(!rm || strcasecmp(rm, "POST")) 
		return;
	cl=getenv("CONTENT_LENGTH");
	if(!cl)
		return;
	content_length=strtol(cl, 0, 0);
#ifndef NDEBUG
	fprintf(stderr, "content_length=%ld\n", content_length);
#endif
        if(content_length > 0 && content_length < MAX_POST_LEN)  {
            char *buf;
            int res;
            buf=malloc(content_length+1);
            res=fread(buf, 1, content_length, ht->post_input);
            if(res<0) {
                perror("fread()");
                exit(EXIT_FAILURE);
            }
			buf[res]=0;
            if(res!=content_length) {
                fprintf(stderr, "Content-Length does not match what was read\n");
                exit(EXIT_FAILURE);
            }
			fprintf(stderr, "calling parse_form_urlencoded @ %d\n", __LINE__);
			parse_form_urlencoded(ht->attr, buf);
            free(buf);

        }
}

static int load_query_string(html_t ht)
{
	const char *qs;
	qs=getenv("QUERY_STRING");
	if(!qs) return 0;
	fprintf(stderr, "calling parse_form_urlencoded @ %d\n", __LINE__);
	parse_form_urlencoded(ht->attr, qs);
	return 1;   
}

html_t htmlcreate(FILE *output, const char  *title)
{
	html_t ret;

#ifndef NDEBUG
	freopen("log", "a", stderr);
	{
		time_t t;
		time(&t);
		fprintf(stderr, "-- %s", ctime(&t));
	}
#endif

	ret=malloc(sizeof *ret);
	ret->output=output;
	ret->post_input=stdin;
	ret->title=strdup(title);
	ret->attr=attrinit();
	ret->cgienv=attrinit();
	load_post_data(ret);
	load_query_string(ret);
	return ret;
}

int htmlvprintf(html_t ht, const char *fmt, va_list ap) 
{
	return vfprintf(ht->output, fmt, ap);
}

int htmlprintf(html_t ht, const char *fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret=htmlvprintf(ht, fmt, ap);
	va_end(ap);
	return ret;
}

void htmlfree(html_t ht)
{
	ht->output=NULL; /* assume this will be closed someplace else */
	free(ht->title);
	attrfree(ht->attr);
	attrfree(ht->cgienv);
	free(ht);
}

void htmlbegin(html_t ht)
{
	htmlprintf(ht, 
		"<HTML>\n"
		"<HEAD>\n");
	
	htmlprintf(ht, " <TITLE>%s</TITLE>\n", ht->title);

	htmlprintf(ht,
		"</HEAD>\n"
		"<BODY>\n");
}

void htmlend(html_t ht)
{
	htmlprintf(ht, "</BODY>\n</HTML>\n");
}

void htmltagbegin ( html_t ht , const char *tag, attrlist_t attr)
{
	if(attr) {
		htmlprintf(ht, "<%s", tag);
		dump_attr(ht, attr);
		htmlprintf(ht, ">");
	} else {
		htmlprintf(ht, "<%s>", tag);
	}
}

void htmltagend ( html_t ht , const char *tag )
{
	htmlprintf(ht, "</%s>", tag);
}

void htmltag(html_t ht, const char *tag, attrlist_t attr, const char *fmt, ...)
{
	va_list ap;
	htmltagbegin(ht, tag, attr);
	if(fmt) {
		va_start(ap,fmt);
		htmlvprintf(ht,fmt,ap);
		va_end(ap);
		htmlprintf(ht,"</%s>\n", tag);
	}
}

void htmlcontent(html_t ht, const char *content_type)
{
	htmlprintf(ht,"Content-Type: %s\n",content_type);
	htmlprintf(ht,"Cache-Control: no-cache\n");
	htmlprintf(ht, "\n");
}

char *htmlmakelink(const char *href, const char  *label, ...)
{
	char *ret;
	int ofs;
	const int max=1024;
	va_list ap;

	va_start(ap,label);
	ret=malloc(max);
	if(!ret) return 0;

	ofs=0;
	ofs+=snprintf(ret+ofs, max-ofs, "<A HREF=\"%s\">", href);
	if(ofs>=max) return ret;
	ofs+=vsnprintf(ret+ofs, max-ofs, label, ap);
	if(ofs>=max) return ret;
	ofs+=snprintf(ret+ofs, max-ofs, "</A>");
	return ret;
}

void htmlhyperlink(html_t ht, const char *href, const char  *label, ...)
{
	va_list ap;
	va_start(ap,label);
	htmlprintf(ht,"<A HREF=\"%s\">",href);
	htmlvprintf(ht,label,ap);
	htmlprintf(ht,"</A>\n");
	va_end(ap);
}

void htmlheader(html_t ht, int level, const char *fmt, ...)
{
	va_list ap;
	va_start(ap,fmt);
	htmlprintf(ht,"<H%d>",level);
	htmlvprintf(ht,fmt,ap);
	htmlprintf(ht,"</H%d>\n",level);
	va_end(ap);
}

const char *htmlget(html_t ht, const char *name)
{
	return attrget(ht->attr,name);
}

attrlist_t htmlattrlist ( html_t ht )
{
	return ht->attr;
}

void htmlcgienvset(html_t ht, const char *name, const char *val)
{
	attrset(ht->cgienv, name, val);
}

