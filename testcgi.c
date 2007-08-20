#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "template.h"
#include "escape.h"

static void template_test(void);
static void escape_test(void);

static void template_test(void) {
	struct template *t;
	attrlist_t al;
	int i;
	const char *files[] = {
		"test1.template",
		"test2.template",
		"test3.template",
	};
	
	al=attrinit();
	attrset(al, "SUBJECT", "[[SUBJECT]]");
	attrset(al, "ICON_URL", "[[ICON_URL]]");
	attrset(al, "DATE", "[[DATE]]");
	attrset(al, "FROM", "[[FROM]]");
	attrset(al, "MESSAGE_BODY", "[[MESSAGE_BODY]]");
	
	/* test with mapped files */
	for(i=0;i<sizeof files/sizeof *files;i++) {
		printf("::%s::\n", files[i]);
		t=template_loadfile(files[i]);
		if(t) {
			template_apply(t, al);
			template_free(t);
		}
		printf("\n");
	}

	/* test with a string */
	printf("::STR::\n");
	/* use a string that shows a bug in the code. $$$ at the end of a string */
	t=template_loadstring("testing $DATE$FROM$MESSAGE_BODY$$$", -1);
	if(t) {
		template_apply(t, al);
		template_free(t);
	}
	printf("\n");

	attrfree(al);
	namefree(); /* free the global list used by attr */
}

static void escape_test(void) {
	const char *t="Hello~!#$%^&(){}<>*/[]=:,;?'\"\\World";
	char *b, *c, *h;
	unsigned len;
	b=uri_escape(0, 0, t, -1);
	c=uri_unescape(0, 0, b, -1);
	len=html_escape_len(t, strlen(t));
	printf("len=%d t.len=%d\n", len, strlen(t));
	h=malloc(len+1);
	html_escape(h, len, t);
	printf("t=\"%s\"\n", t);
	printf("b=\"%s\"\n", b);
	printf("c=\"%s\"\n", c);
	printf("h=\"%s\"\n", h);
	printf("uri_escape() and uri_unescape(): ");
	if(!strcmp(t, c)) {
		printf("Passed\n");
	} else {
		printf("FAILED\n");
	}
	free(h);
	free(b);
	free(c);
}

int main(int argc, char **argv) {
	template_test();
	escape_test();
	return 0;
}
