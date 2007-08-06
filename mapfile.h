#ifndef MAPFILE_H
#define MAPFILE_H
/* mapfile.c */
#ifdef WIN32
struct mapfile_info {
	const char *data; /* I'd like to assert that the file is read-only */
	size_t len;
	HANDLE hmap;
};

const char *mapfile_handle(struct mapfile_info *mi, HANDLE hf, const char *hf_name);
#else
/* POSIX */
struct mapfile_info {
	const char *data; /* I'd like to assert that the file is read-only */
	size_t len;
};

const char *mapfile_fd(struct mapfile_info *mi, int fd, const char *fd_name);
#endif

const char *mapfile(struct mapfile_info *mi, const char *filename);
void mapfile_release(struct mapfile_info *mi);
#endif
