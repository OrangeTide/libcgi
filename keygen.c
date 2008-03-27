/* TODO: use /dev/random to gather entropy */

#include <stdio.h>
#include <stdlib.h>
#include "keygen.h"

/* generates a random string between lower and upper characters */
char *random_string(unsigned lower, unsigned upper) {
        char *ret;
        unsigned i, len;
		if(upper>lower) {
			len=rand()%(upper-lower)+lower;
		} else {
			/* refuse to do modulus/remainder on zero or negative numbers */
			len=lower;
		}
        ret=malloc(len+1);
        if(!ret) { /* verify the allocation */
                perror("malloc()");
                return 0;
        }
        for(i=0;i<len;i++) {
                ret[i]=(rand()%95)+' ';
        }
        ret[i]=0;
        return ret;
}
