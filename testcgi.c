#include <stdio.h>
#include "template.h"

int main(int argc, char **argv) {
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
	return 0;
}
