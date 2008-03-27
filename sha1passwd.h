/* sha1passwd.h - hashes passwords using SHA1
 * $Id: sha1passwd.c 156 2007-07-12 23:29:10Z orange $
 */
#ifndef SHA1PASSWD_H
#define SHA1PASSWD_H
#include <stddef.h>
#include <openssl/sha.h>
#define SSHA1_SALT_MAX 64
#define SSHA1_MAGIC_LENGTH 7
#define SSHA1_MAGIC "$ssha1$"
#define SSHA1_CRYPTTEXT_MAX (((SSHA1_SALT_MAX+SHA_DIGEST_LENGTH)*4/3)+SSHA1_MAGIC_LENGTH)
void ssha1_newpass(const char *cleartext, size_t out_len, char *out);
int ssha1_checkpass(const char *cleartext, const char *crypttext);
int ssha1_isvalid(const char *ciphertext);
int ssha1_getsalt(const char *ciphertext, size_t salt_len, unsigned char *salt);
#endif
