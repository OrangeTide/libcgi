attr.c
======

Manages a list of name-value pairs.

void attrcatn(attrlist_t al, const char *name, const _char *value, size_t len);
void attrcat(attrlist_t al, const char *name, const _char *value);
void attrsetn(attrlist_t al, const char *name, const _char *value, size_t len);
void attrset(attrlist_t al, const char *name, const _char *value);
void attrset_safe(attrlist_t al, const char *name, const _char *value);
int attrprintf(attrlist_t al, const char *name, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));
int attrvprintf(attrlist_t al, const char *name, const char *fmt, va_list ap);
const _char *attrget(attrlist_t al, const char *name);
int attrlist(attrlist_t al, const _char **type, const _char **value, int *counter);
attrlist_t attrinit(void);
void attrfree(attrlist_t al);
void namefree(void);
int attrget_int(attrlist_t al, const char *name, long *i);



base64.c
========

/* used to encode 3 bytes into 4 base64 digits */
void base64encode(const unsigned char in[3], unsigned char out[4], int count);

/* used to decode 4 base64 digits into 3 bytes */
int base64decode(const char in[4], char out[3]);

/* encode binary data into base64 digits with MIME style === pads */
int base64_encode(size_t in_len, const unsigned char *in, size_t out_len, char *out);

/* decode base64 digits with MIME style === pads into binary data */
int base64_decode(size_t in_len, const char *in, size_t out_len, unsigned char *out);

cgi.c
=====

Man CGI interface. Does additional error checking beyond using printf.
It also provides an abstraction that makes it easier to do embedded webservers
with the CGI functionality built-in.

cgi_t cgi_init(void);
int cgi_vprintf(cgi_t ht, const char *fmt, va_list ap);
int cgi_printf(cgi_t ht, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
void cgi_set_content_type(cgi_t ht, const char *content_type);
void cgi_set_cache_control(cgi_t ht, const char *cache_control);
void cgi_start_headers(cgi_t ht);
void cgi_setparam(cgi_t ht, const char *name, const _char *val);
const _char *cgi_param(cgi_t ht, const char *name);
attrlist_t cgi_attrlist(cgi_t ht);
void cgi_free(cgi_t ht);
int cgi_param_int(cgi_t c, const char *name, long *i);
int cgi_set_cookie(cgi_t c, const char *name, const char *fmt, ...) __attribute__( (format (printf, 3, 4)));
void cgi_set_cookie_expires(cgi_t c, time_t value);
void cgi_set_cookie_domain(cgi_t c, const char *value);
void cgi_set_cookie_path(cgi_t c, const char *value);
void cgi_set_cookie_secure(cgi_t c, int secure);
const _char *cgi_cookie(cgi_t c, const char *name);
int cgi_cookie_int(cgi_t c, const char *name, long *i);
const char *home_filename(const char *home_dir, const char *filename);


escape.c
========

Encode and decode URI and HTML style escapes.

unsigned uri_escape_len(const _char *s, size_t len);
char *uri_escape(char *dest, size_t dest_len, const char *src, int src_len);
char *uri_unescape(char *dest, size_t dest_len, const _char *src, int src_len);
unsigned html_escape_len(const char *s, size_t len);
void html_escape(char *dest, size_t len, const char *s);

ini.c
=====

Parse INI like files.
	[section name]
	name=value

void ini_free(struct ini_info *ii);
	Destroy the struct ini_info structure.

struct ini_info *ini_load(const char *filename);
	Read a file into a newly created struct ini_info structure.
	Start section cursor at the first section.
	File is read completely by this function and does not remain open.
	Return NULL on error.

const char *ini_next_section(struct ini_info *ii);
	Move section cursor to next section.
	Return name of section.
	Return NULL on end of file.

const char *ini_next_parameter(struct ini_info *ii, const char **parameter);
	Read the next name-value pair from the current section.
	Return NULL on end of section.

void ini_rewind(struct ini_info *ii);
	Go back to the first section of the file.

const char *ini_get(struct ini_info *ii, const char *section, const char *parameter);
	Search for a particular entry, does not alter the section cursor.
	Return value of (section, name) tuple.
	Return NULL if (section, name) tuple not found.


keygen.c
========

Generate a random string useful for session keys.

char *random_string(unsigned lower, unsigned upper, const char *charset);
	lower - minimum length of string.
	upper - maximum length of string
	charset - null terminated string to use for set, determines "base".

	If charset is NULL then use A-Za-z0-9+/ for a 64 character set.


mapfile.c
=========

Wrapper to memory map a file.

const char *mapfile_handle(struct mapfile_info *mi, HANDLE hf, const char *hf_name);
	WIN32 only. Map an entire file based on a handle.

const char *mapfile_fd(struct mapfile_info *mi, int fd, const char *fd_name);
	POSIX hosts only. Map an entire file based on file descriptor.
	Must be a disk file or device fd, cannot be a socket.

const char *mapfile(struct mapfile_info *mi, const char *filename);
	Map an entire file to memory.
	mi should be unitialized, and is initialized by this function.

void mapfile_release(struct mapfile_info *mi);
	Release a mapping.
	mi must have been initialized by mapfile().



sha1passwd.c
============

Routines to implemented Salted SHA1 password hashes.

void ssha1_newpass(const char *cleartext, size_t out_len, char *out);
int ssha1_checkpass(const char *cleartext, const char *crypttext);
int ssha1_isvalid(const char *ciphertext);
int ssha1_getsalt(const char *ciphertext, size_t salt_len, unsigned char *salt);

template.c
=========

Processes template files and performs substitutions on them. Substitutions are in the form of ${foo}. Uses attr-list to configure the substitutions.

typedef struct template template_t;
template_t *template_loadfile(const char *filename);
	Create a template from a file. Uses mapfile()
	File will remain open and mapped until template_free()

template_t *template_loadstring(const char *str, int len);
	Create a template from a string.
	str parameter will be used until template_free() is called.
	no allocation or duplication will occur on str.

void template_apply(template_t *t, attrlist_t al);
	output data to stdout, with appropriate substitutions.

void template_free(template_t *t);
	release data.

testcgi.c
=========

Example CGI application using some of these modules.
