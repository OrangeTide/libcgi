/* mapfile.c : PUBLIC DOMAIN - Jon Mayo - August 11, 2006
 * - You may remove any comments you wish, modify this code any way you wish,
 *   and distribute any way you wish.*/
/* allows a file to mapped into memory (read-only) to be used as one big string.
 * NOTE: the file is not null terminated. you must use the len parameter
 * on 32-bit systems you may have trouble loading very large files (like
 * around 512M - 1GB). A more elegant solution would be to map around 1MB at a
 * time and just move the map forward with a new offset.  */
#include <assert.h> /* compile with NDEBUG defined to disable asserts() */
#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#else
/* assume unix/posix if not WIN32 */
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "mapfile.h"

/** put in mapfile.c **/

#ifdef WIN32
/* Win32 Version */
static void show_error(const char *reason) {
	LPTSTR lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &lpMsgBuf, 0, NULL);
	if(reason) {
		fprintf(stderr, "%s:%s\n", reason, lpMsgBuf);
	} else {
		fprintf(stderr, "%s\n", lpMsgBuf);
	}
	LocalFree(lpMsgBuf);
}

const char *mapfile_handle(struct mapfile_info *mi, HANDLE hf, const char *hf_name) {
	HANDLE hmap;
	/* maybe we should use SEC_RESERVE instead. should we use hf_name? */
	hmap=CreateFileMapping(hf, NULL, PAGE_READONLY|SEC_COMMIT, 0, 0, NULL);
	if(hmap<=0) {
		show_error(hf_name);
		return 0;
	}
	mi->data=MapViewOfFile(hmap, FILE_MAP_READ, 0, 0, 0);
	mi->len=GetFileSize(hf, 0);
	return mi->data;
}

const char *mapfile(struct mapfile_info *mi, const char *filename) {
	HANDLE hfile;
	const char *ret;
	hfile=CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,  0);
	if(hfile==INVALID_HANDLE_VALUE) {
		show_error(filename);
		return 0;
	}
	ret=mapfile_handle(mi, hfile, filename);
    CloseHandle(hfile);
	return ret;
}

void mapfile_release(struct mapfile_info *mi) {
	if(!UnmapViewOfFile((LPVOID)mi->data)) {
		show_error("UnmapViewOfFile()");
	}
	CloseHandle(mi->hmap);
}

#else
/* POSIX version */

/* mi : a handle to "free" later. does not need to be initialized
 * fd : file descriptor
 * fd_name : name to put in error messages
 * returns a pointer to the filedata, NULL on failure */
const char *mapfile_fd(struct mapfile_info *mi, int fd, const char *fd_name) {
	void *ptr;
	struct stat st;
	assert(mi != NULL);
	if(fstat(fd, &st)) {
		perror(fd_name);
		return NULL;
	}
	/* mmap() according to susv3 : "while the argument len need not meet a
	 * size or alignment constraint, the implementation shall include, in
	 * any mapping operation, any partial page specified by the range
	 * [pa,pa+len)." */
	ptr=mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if(ptr==MAP_FAILED) {
		perror(fd_name);
		return NULL;
	}
	mi->len=st.st_size;
	mi->data=ptr;
	return ptr;
}

/* mi : a handle to "free" later. does not need to be initialized
 * filename : filename to open (as read-only)
 * returns a pointer to the filedata, NULL on failure */
const char *mapfile(struct mapfile_info *mi, const char *filename) {
	int fd;
    const char *ret;
    assert(filename != NULL);
	fd=open(filename, O_RDONLY);
    if(fd<0) {
            perror(filename);
            return NULL;
    }
    ret=mapfile_fd(mi, fd, filename);
    close(fd);
    return ret;
}

/* mi : pointer to the mapfile_info handle */
void mapfile_release(struct mapfile_info *mi) {
    assert(mi != NULL);
    assert(mi->data != NULL);
    assert(mi->len > 0);
    if(munmap((void*)mi->data, mi->len)) {
            perror("munmap()");
    }
}
#endif

/* define to try the example code below */
#ifdef STAND_ALONE
#include <stdlib.h>

/* just pass a list of filename to test this out on */
int main(int argc, char **argv) {
    unsigned i;
    for(i=1;i<argc;i++) {
        struct mapfile_info mi; /* notice it's not a pointer */
        if(mapfile(&mi, argv[i])) {
            /* this code just does an xor8 style checksum */
            unsigned char sum8=~0;
            unsigned j;
            for(j=0;j<mi.len;j++) {
                sum8^=mi.data[j];
            }
            mapfile_release(&mi);
            printf("sum8(%s): 0x%02x\n", argv[i], sum8);
        } else {
            return EXIT_FAILURE;
        }
    }
    return 0;
}
#endif
