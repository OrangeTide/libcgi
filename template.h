#ifndef TEMPLATE_H
#define TEMPLATE_H
#include <stdio.h>
#include "attr.h"
struct template;
struct template *template_loadfile(const char *filename);
void template_apply(struct template *t, attrlist_t al, FILE *out);
void template_free(struct template *t);
#endif
