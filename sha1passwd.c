/* sha1passwd.c - hashes passwords using SHA1
 * PUBLIC DOMAIN - Jon Mayo - July 8, 2006
 * $Id: sha1passwd.c 156 2007-07-12 23:29:10Z orange $
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include "sha1passwd.h"
#include "base64.h"

#define SALT_GEN_SIZE 8

static int ssha1_getdata(const char *ciphertext, size_t data_len, unsigned char *data) {
	size_t len;

	ciphertext+=SSHA1_MAGIC_LENGTH;
	len=base64_decode(strlen(ciphertext), ciphertext, data_len, data);
	if(len<=SHA_DIGEST_LENGTH) return 0; /* failure */
	if(len>data_len) return 0; /* failure */
	return len;
}

static int ssha1_cryptdata(const char *cleartext, size_t salt_len, const void *salt, size_t out_len, void *out) {
	SHA_CTX ctx;
	size_t len;
	len=(SHA_DIGEST_LENGTH+salt_len);
	if(out_len<len) return 0;

	SHA1_Init(&ctx);
	SHA1_Update(&ctx, (const unsigned char*)cleartext, strlen(cleartext));
	SHA1_Update(&ctx, salt, salt_len);
	SHA1_Final(out, &ctx);
	memcpy((char*)out+SHA_DIGEST_LENGTH, salt, salt_len);
	return len;
}

static int ssha1_crypt(const char *cleartext, size_t salt_len, const unsigned char *salt, size_t out_len, char *out) {
	unsigned char cipher[SHA_DIGEST_LENGTH + SSHA1_SALT_MAX];
	int len;
	len=ssha1_cryptdata(cleartext, salt_len, salt, sizeof cipher, cipher);
	strcpy(out, SSHA1_MAGIC);
	len=base64_encode(SHA_DIGEST_LENGTH + salt_len, cipher, out_len-SSHA1_MAGIC_LENGTH, out+SSHA1_MAGIC_LENGTH);
	return len;
}

static void ssha1_gensalt(size_t salt_len, void *salt) {
	size_t i;
	for(i=0;i<salt_len;i++) {
		/* TODO: use better random salt */
		((unsigned char*)salt)[i]=(rand()%96)+' ';
	}
}

void ssha1_newpass(const char *cleartext, size_t out_len, char *out) {
	unsigned char salt[SALT_GEN_SIZE];
	ssha1_gensalt(sizeof salt, salt);
	ssha1_crypt(cleartext, sizeof salt, salt, out_len, out);
}

int ssha1_checkpass(const char *cleartext, const char *crypttext) {
	unsigned char data[SHA_DIGEST_LENGTH + SSHA1_SALT_MAX];
	size_t data_len;
	unsigned char check[SHA_DIGEST_LENGTH + SSHA1_SALT_MAX];
	size_t check_len;

	data_len=ssha1_getdata(crypttext, sizeof data, data);
	if(!data_len) return 0;

	check_len=ssha1_cryptdata(cleartext, data_len - SHA_DIGEST_LENGTH, data+SHA_DIGEST_LENGTH, sizeof check, check);
	if(!check_len) return 0;
	if(check_len!=data_len) return 0;
	return memcmp(data, check, check_len)==0;
}

/* UNTESTED */
int ssha1_isvalid(const char *ciphertext)
{
	return !strncmp(ciphertext, SSHA1_MAGIC, SSHA1_MAGIC_LENGTH);
}

/* utility function */
int ssha1_getsalt(const char *ciphertext, size_t salt_len, unsigned char *salt) {
	size_t len;

	unsigned char res[SHA_DIGEST_LENGTH+SSHA1_SALT_MAX];

	if(!ssha1_isvalid(ciphertext)) return 0;

	ciphertext+=SSHA1_MAGIC_LENGTH;
	len=base64_decode(strlen(ciphertext), ciphertext, sizeof res, res);
	/* TODO: check result */
	if(len<=SHA_DIGEST_LENGTH) return 0; /* failure */
	len-=SHA_DIGEST_LENGTH;
	if(len>salt_len) return 0; /* failure */
	if(len>SSHA1_SALT_MAX) len=SSHA1_SALT_MAX;
	memcpy(salt, res+SHA_DIGEST_LENGTH, len<salt_len?len:salt_len);
	return len;
}

/* Testing code */
#ifdef STAND_ALONE
#include <unistd.h>
#include <ctype.h>

static int verbose=0;

static void do_test(void) {
	static const struct {
		char *sha1passwd;
		char *cleartext;
	} tests[] = {
		{"$ssha1$/EExmSfmhQSPHDJaTxwQSdb/uPpzYWx0ZXI=", "secret"},
		{"$ssha1$gVK8WC9YyFT1gMsQHTGCgT3sSv5zYWx0", "secret"},
		{"$ssha1$W3ipFGmzS3+j6/FhT7ZC39MIfqFcct9Ep0KEGA==", "asddsa123"},
		{"$ssha1$YbB2R1D2AlzYc9wk/YPtslG7NoiOWaoMOztLHA==", "ripthispassword"},
		{"$ssha1$0jVwy2q3GhzwzqAPTLzsWqGJEOsnZikzMT8qLA==", "foo"},
		{NULL, NULL}
	};

	int i;
	for(i=0;tests[i].sha1passwd;i++) {
		unsigned char salt[SSHA1_SALT_MAX];
		unsigned salt_len;
		char bleh[SSHA1_CRYPTTEXT_MAX];
		printf("orig : %s\n", tests[i].sha1passwd);
		if(!(salt_len=ssha1_getsalt(tests[i].sha1passwd, sizeof salt, salt))) {
			fprintf(stderr, "failure\n");
		}
		ssha1_crypt(tests[i].cleartext, salt_len, salt, sizeof bleh, bleh);
		fprintf(stderr, "check: %s\n", bleh);

	}
}

static void usage(void) {
	printf(
	"usage: sha1passwd [-ht] [-c hash] [-s hash] [passwords...]\n"
	"  -c    check password(s) against hash. return success if any match.\n"
	"  -s    get salt from hashed password\n"
	"  -h    help\n"
	"  -v    verbose\n"
	"  -t    perform tests\n"
	);
}

int main(int argc, char **argv) {
	const char *check_pass=0; /* set to enable password checking */
	while(1) {
		int c=getopt(argc, argv, "htvc:s:");
		if(c==-1) break;
		switch(c) {
			case 'h':
				usage();
				exit(EXIT_FAILURE);
			case 't':
				do_test();
				exit(EXIT_FAILURE);
			case 'c': /* check */
				if(check_pass) {
					fprintf(stderr, "WARNING: only one password check supported at a time.\n");
				}
				check_pass=optarg;
				break;
			case 's': { /* get salt */
				unsigned char salt[256]; /* support really big salts */
				int salt_len;

				salt_len=ssha1_getsalt(optarg, sizeof salt, salt);
				if(salt_len>=0) {
					int i;
					printf("salt: ");
					for(i=0;i<salt_len;i++) {
						printf("%02X", salt[i]);
					}
					printf(" (len=%u)\n", salt_len);
				} else {
					printf("salt error.\n");
					exit(EXIT_FAILURE);
				}
				break;
			}
			case 'v':
				verbose++;
				break;
			default:
				fprintf(stderr, "Unknown option '%c'\n", isprint(c)?c:'?');
				usage();
				exit(EXIT_FAILURE);
		}
	}
	if(check_pass) {
		/* check the password */
		int ret=0;
		while(optind<argc) {
			int res;
			res=ssha1_checkpass(argv[optind], check_pass);
			if(verbose>1) {
				if(res) {
					fprintf(stderr, "Check Passed.\n");
				} else {
					fprintf(stderr, "Check Failed.\n");
				}
			}
			if(res) {
				ret++;
			}
			optind++;
		}
		if(!ret) {
			if(verbose>0) {
				fprintf(stderr, "Failed, no passwords matched.\n");
			}
			return EXIT_FAILURE; /* no passwords matched */
		} else {
			if(verbose>0) {
				fprintf(stderr, "Success.\n");
			}
			return EXIT_SUCCESS; /* password matched */
		}
	} else {
		/* encode a password */
		while(optind<argc) {
			char out[SSHA1_CRYPTTEXT_MAX];
			/* TODO: check for errors */
			ssha1_newpass(argv[optind], sizeof out, out);
			printf("%s\n", out);
		optind++;
		}
	}

	return EXIT_SUCCESS;
}
#endif /* testing code */
/* vim: ts=4 sts=0 noet sw=4
*/
