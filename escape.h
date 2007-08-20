#ifndef ESCAPE_H
#define ESCAPE_H
#include "attr.h"
unsigned uri_escape_len(const _char *s, size_t len);
char *uri_escape(char *dest, size_t dest_len, const char *src, int src_len);
char *uri_unescape(char *dest, size_t dest_len, const _char *src, int src_len);
unsigned html_escape_len(const char *s, size_t len);
void html_escape(char *dest, size_t len, const char *s);
#endif
