/* keygen.c - generate random strings. */
/* PUBLIC DOMAIN - Jon Mayo - March 27, 2008 */
/* TODO: use /dev/random to gather entropy */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "keygen.h"

/* generates a random string between lower and upper characters */
char *random_string(unsigned lower, unsigned upper, const char *charset) {
        char *ret;
        unsigned i, len;
		size_t charset_len;
		if(upper>lower) {
			len=rand()%(upper-lower)+lower;
		} else {
			/* refuse to do modulus/remainder on zero or negative numbers */
			len=lower;
		}
		if(!charset) {
			charset="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		}
		charset_len=strlen(charset);
		if(charset_len<=0) {
			return NULL; /* failure */
		}
        ret=malloc(len+1);
        if(!ret) { /* verify the allocation */
                perror("malloc()");
                return 0;
        }
        for(i=0;i<len;i++) {
                ret[i]=charset[rand()%charset_len];
        }
        ret[i]=0;
        return ret;
}
