#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "cgi.h"
#include "attr.h"

struct html_t
{
	FILE *output;
	char *title;
	attrlist_t attr;
};
int htmlcgiquery(html_t ht);

html_t htmlcreate(FILE *output, const char  *title)
{
	html_t ret;
	ret=malloc(sizeof *ret);
	ret->output=output;
	ret->title=strdup(title);
	ret->attr=attrinit();
	htmlcgiquery(ret);
	return ret;
}

void htmlfree(html_t ht)
{
	ht->output=NULL; /* assume this will be closed someplace else */
	free(ht->title);
	free(ht);
}

void htmlbegin(html_t ht)
{
	fprintf(ht->output,
		"<HTML>\n"
		"<HEAD>\n");
	
	fprintf(ht->output, " <TITLE>%s</TITLE>\n", ht->title);

	fprintf(ht->output,
		"</HEAD>\n"
		"<BODY>\n");
}

void htmlparagraph(html_t ht, const char *fmt, ...)
{
	va_list ap;
	va_start(ap,fmt);
	fprintf(ht->output,"<P>\n");
	vfprintf(ht->output,fmt,ap);
	fprintf(ht->output,"\n</P>\n");
	va_end(ap);
}

void htmlend(html_t ht)
{
	fprintf(ht->output, "</BODY>\n</HTML>\n");
}

void htmllistbegin(html_t ht)
{
	fprintf(ht->output,"<UL>\n");
}

void htmllist(html_t ht, const char *fmt, ...)
{
	va_list ap;
	va_start(ap,fmt);
	fprintf(ht->output," <LI>");
	vfprintf(ht->output,fmt,ap);
	fprintf(ht->output,"</LI>\n");
	va_end(ap);
}
void htmllistend(html_t ht)
{
	fprintf(ht->output,"</UL>\n");
}

void htmlcontent(html_t ht, const char *content_type)
{
	fprintf(ht->output,"Content-Type: %s\n\n",content_type);
}

void htmlhyperlink(html_t ht, const char *href, const char  *label, ...)
{
	va_list ap;
	va_start(ap,label);
	fprintf(ht->output,"<A HREF=\"%s\">",href);
	vfprintf(ht->output,label,ap);
	fprintf(ht->output,"</A>\n");
	va_end(ap);
}

void htmlheader(html_t ht, int level, const char *fmt, ...)
{
	va_list ap;
	va_start(ap,fmt);
	fprintf(ht->output,"<H%d>",level);
	vfprintf(ht->output,fmt,ap);
	fprintf(ht->output,"</H%d>\n",level);
	va_end(ap);
}

const char *htmlget(html_t ht, const char *name)
{
	return attrget(ht->attr,name);
}

void htmlset(html_t ht, const char *name, const char *val)
{
	int len=strlen(name)+1;
	char n[len]; /* C99 variable length arrays */
	int i;
	for(i=0;name[i];i++)
	{
		n[i]=tolower(name[i]); /* converts it to lowercase */
	}
	n[i]=0;
	attrset(ht->attr,n,val);
}

const char *query_string(void)
{
   return getenv("QUERY_STRING");
}

int htmlcgiquery(html_t ht)
{
	const char *qs;
	char namebuf[1024]; /* buffer to hold the current name element */
	char valuebuf[1024]; /* buffer to hold the current value */
	const char *headp; 
	const char *tailp;
	int len;
	qs=query_string(); 
	if(!qs) return 0;
	for(headp=qs;*headp;) {
		tailp=headp+strcspn(headp,"=&");
		len=tailp-headp<sizeof namebuf?tailp-headp:sizeof namebuf-1;
		if(len) memcpy(namebuf,headp,len);
		namebuf[len]=0;
		if(!tailp[0]) { valuebuf[0]=0; htmlset(ht,namebuf,valuebuf); break; }
		headp=tailp+1;
		if(tailp[0]=='&') { valuebuf[0]=0; htmlset(ht,namebuf,valuebuf); continue; }
		tailp=headp+strcspn(headp,"&");
		len=tailp-headp<sizeof valuebuf?tailp-headp:sizeof valuebuf-1;
		if(len) memcpy(valuebuf,headp,len);
		valuebuf[len]=0;
		if(!tailp[0]) { htmlset(ht,namebuf,valuebuf); break; }
		headp=tailp+1;
		htmlset(ht,namebuf,valuebuf);
	}
	return 1;   
}

