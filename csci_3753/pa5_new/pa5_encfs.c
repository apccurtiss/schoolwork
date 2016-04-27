/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  Minor modifications and note by Andy Sayler (2012) <www.andysayler.com>
  
  Additional modifications by Alexander Curtiss (2016) <www.alexandercurtiss.com>

  Source: fuse-2.8.7.tar.gz examples directory
  http://sourceforge.net/projects/fuse/files/fuse-2.X/

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  Note: This implementation is largely stateless and does not maintain
        open file handels between open and release calls (fi->fh).
        Instead, files are opened and closed as necessary inside read(), write(),
        etc calls. As such, the functions that rely on maintaining file handles are
        not implmented (fgetattr(), etc). Those seeking a more efficient and
        more complete implementation may wish to add fi->fh support to minimize
        open() and close() calls and support fh dependent functions.

  References: http://www.cs.nmsu.edu/~pfeiffer/fuse-tutorial/

*/


#define FUSE_USE_VERSION 28
#define HAVE_SETXATTR

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#include <stdlib.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#include <openssl/evp.h>
#include <openssl/aes.h>
#include "aes-crypt.h"

#define BLOCKSIZE 1024
#define FAILURE 0
#define SUCCESS 1
#define WRITE 0
#define READ 1

#define private_data ((struct inputs*)fuse_get_context()->private_data)
#define MAX_FILEPATH_LENGTH 1024

struct inputs {
	char* filepath;
	char* keyphrase;
};

void get_filepath(char location[MAX_FILEPATH_LENGTH], const char* relative_path) {
	strcpy(location, private_data->filepath);
	strncat(location, relative_path, MAX_FILEPATH_LENGTH);
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
	int res;
	char full_path[MAX_FILEPATH_LENGTH];
	get_filepath(full_path, path);

	printf("getAttr\n");
	res = lstat(full_path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_access(const char *path, int mask)
{
	int res;
	char full_path[MAX_FILEPATH_LENGTH];
	get_filepath(full_path, path);

	res = access(full_path, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
{
	int res;
	char full_path[MAX_FILEPATH_LENGTH];
	get_filepath(full_path, path);

	res = readlink(full_path, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}


static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	char full_path[MAX_FILEPATH_LENGTH];
	get_filepath(full_path, path);

	dp = opendir(full_path);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0))
			break;
	}

	closedir(dp);
	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;
	char full_path[MAX_FILEPATH_LENGTH];
	get_filepath(full_path, path);

	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(full_path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(full_path, mode);
	else
		res = mknod(full_path, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	int res;
	char full_path[MAX_FILEPATH_LENGTH];
	get_filepath(full_path, path);

	res = mkdir(full_path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_unlink(const char *path)
{
	int res;
	char full_path[MAX_FILEPATH_LENGTH];
	get_filepath(full_path, path);

	res = unlink(full_path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path)
{
	int res;
	char full_path[MAX_FILEPATH_LENGTH];
	get_filepath(full_path, path);

	res = rmdir(full_path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_symlink(const char *from, const char *to)
{
	int res;
	char full_path_from[MAX_FILEPATH_LENGTH];
	get_filepath(full_path_from, from);

	char full_path_to[MAX_FILEPATH_LENGTH];
	get_filepath(full_path_to, to);

	res = symlink(full_path_from, full_path_to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rename(const char *from, const char *to)
{
	int res;
	char full_path_from[MAX_FILEPATH_LENGTH];
	get_filepath(full_path_from, from);

	char full_path_to[MAX_FILEPATH_LENGTH];
	get_filepath(full_path_to, to);
	res = rename(full_path_from, full_path_to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_link(const char *from, const char *to)
{
	int res;
	char full_path_from[MAX_FILEPATH_LENGTH];
	get_filepath(full_path_from, from);

	char full_path_to[MAX_FILEPATH_LENGTH];
	get_filepath(full_path_to, to);

	res = link(full_path_from, full_path_to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chmod(const char *path, mode_t mode)
{
	int res;
	char full_path[MAX_FILEPATH_LENGTH];
	get_filepath(full_path, path);

	res = chmod(full_path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{
	int res;
	char full_path[MAX_FILEPATH_LENGTH];
	get_filepath(full_path, path);

	res = lchown(full_path, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	int res;
	char full_path[MAX_FILEPATH_LENGTH];
	get_filepath(full_path, path);

	res = truncate(full_path, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	int res;
	struct timeval tv[2];
	char full_path[MAX_FILEPATH_LENGTH];
	get_filepath(full_path, path);

	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;

	res = utimes(full_path, tv);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	(void) fi;

	int res;
	char full_path[MAX_FILEPATH_LENGTH];
	char temp_path[MAX_FILEPATH_LENGTH];
	char full_temp_path[MAX_FILEPATH_LENGTH];

	//  Get actual filepath
	get_filepath(full_path, path);
	//  Get path of a temp file (temp_[file name]) that will store the old, encrypted version
	strcpy(temp_path, "temp_");
	strcat(temp_path, path);
	get_filepath(full_temp_path, temp_path);
	strcpy(full_temp_path, "/home/user/tempfile");

	res = open(full_path, fi->flags);
	if (res == -1)
		return -errno;
	close(res);

    FILE* original = fopen(full_path, "rb");
    if(!original){
		perror("file open error\n");
		return 0;
    }

    FILE* temp = fopen("/tmp/test", "w");
    if(!temp){
		perror("temp open error\n");
		return 0;
    }

	//  Decrypt original into temp
	do_crypt(original, temp, 0, private_data->keyphrase);

    if(fclose(original)){
        perror("original close error\n");
    }
    if(fclose(temp)){
		perror("temp close error\n");
    }

	//  Reopen files in reverse
    original = fopen(full_path, "w");
    if(!original){
		system("touch /tmp/file_open_error");
		perror("file open error\n");
		return 0;
    }

    temp = fopen("/tmp/test", "rb");
    if(!temp){
		system("touch /tmp/temp_open_error");
		perror("temp open error\n");
		return 0;
    }

	//  Copy temp back into original
	do_crypt(temp, original, -1, private_data->keyphrase);
	system("touch /tmp/decrypted");

    if(fclose(original)){
        perror("original close error\n");
    }
    if(fclose(temp)){
		perror("temp close error\n");
    }

	return 0;
}

//rtag
static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	char full_path[MAX_FILEPATH_LENGTH];
	get_filepath(full_path, path);
	int fd; 
	int res;

	(void) fi; 

	fd = open(full_path, O_RDONLY);
	if (fd == -1) 
		return -errno;

	res = pread(fd, buf, size, offset);

	if (res == -1) 
		res = -errno;
	close(fd);

	return res;
}
    

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;
	char full_path[MAX_FILEPATH_LENGTH];
	get_filepath(full_path, path);

	(void) fi;
	fd = open(full_path, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	int res;
	char full_path[MAX_FILEPATH_LENGTH];
	get_filepath(full_path, path);

	res = statvfs(full_path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_create(const char* path, mode_t mode, struct fuse_file_info* fi) {

    (void) fi;

    int res;
	char full_path[MAX_FILEPATH_LENGTH];
	get_filepath(full_path, path);

    res = creat(full_path, mode);
    if(res == -1)
	return -errno;

    close(res);

    return 0;
}


static int xmp_release(const char *path, struct fuse_file_info *fi)
{
	(void) fi;

	char full_path[MAX_FILEPATH_LENGTH];
	char temp_path[MAX_FILEPATH_LENGTH];
	char full_temp_path[MAX_FILEPATH_LENGTH];

	//  Get actual filepath
	get_filepath(full_path, path);

	//  Get path of a temp file (~[file name]) that will store the unencrypted version
	strcpy(temp_path, "~");
	strcat(temp_path, path);
	get_filepath(full_temp_path, temp_path);

    FILE* original = fopen(full_path, "rb");
    if(!original){
		perror("file open error\n");
		return 0;
    }
    FILE* temp = fopen(full_temp_path, "bw+");
    if(!temp){
		perror("temp open error\n");
		return 0;
    }

	//  Copy file into temp
	do_crypt(original, temp, -1, private_data->keyphrase);

	//  Encrypt temp into original
	do_crypt(temp, original, 1, private_data->keyphrase);

	
    if(fclose(original)){
        perror("original close error\n");
    }
    if(fclose(temp)){
		perror("temp close error\n");
    }
	
	int ret = remove(full_temp_path);
	if (ret != 0) {
		perror("Error deleting temp\n");
	}

	return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}

#ifdef HAVE_SETXATTR
static int xmp_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{
	char full_path[MAX_FILEPATH_LENGTH];
	get_filepath(full_path, path);
	int res = lsetxattr(full_path, name, value, size, flags);
	if (res == -1)
		return -errno;
	return 0;
}

static int xmp_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
	char full_path[MAX_FILEPATH_LENGTH];
	get_filepath(full_path, path);
	int res = lgetxattr(full_path, name, value, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_listxattr(const char *path, char *list, size_t size)
{
	char full_path[MAX_FILEPATH_LENGTH];
	get_filepath(full_path, path);
	int res = llistxattr(full_path, list, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_removexattr(const char *path, const char *name)
{
	char full_path[MAX_FILEPATH_LENGTH];
	get_filepath(full_path, path);
	int res = lremovexattr(full_path, name);
	if (res == -1)
		return -errno;
	return 0;
}
#endif /* HAVE_SETXATTR */

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.access		= xmp_access,
	.readlink	= xmp_readlink,
	.readdir	= xmp_readdir,
	.mknod		= xmp_mknod,
	.mkdir		= xmp_mkdir,
	.symlink	= xmp_symlink,
	.unlink		= xmp_unlink,
	.rmdir		= xmp_rmdir,
	.rename		= xmp_rename,
	.link		= xmp_link,
	.chmod		= xmp_chmod,
	.chown		= xmp_chown,
	.truncate	= xmp_truncate,
	.utimens	= xmp_utimens,
	.open		= xmp_open,
	.read		= xmp_read,
	.write		= xmp_write,
	.statfs		= xmp_statfs,
	.create     = xmp_create,
	.release	= xmp_release,
	.fsync		= xmp_fsync,
#ifdef HAVE_SETXATTR
	.setxattr	= xmp_setxattr,
	.getxattr	= xmp_getxattr,
	.listxattr	= xmp_listxattr,
	.removexattr	= xmp_removexattr,
#endif
};


int main(int argc, char *argv[])
{
    //  Fix loose nut errors
    if (argc < 4) {
        printf("Usage: ./pa5-encfs <key phrase> <mirror directory> <mount point>\n");
        exit(1);
    }

    //  Create private data to be passed to fuse functions
    struct inputs data;
    data.filepath = realpath(argv[2], NULL);
    data.keyphrase = argv[1];

    //  Permissions for new files will be 000
    umask(0);

    //  Prepare arguments to be passed to fuse by placing the mount point as argv[1]
    argv[1] = argv[3];
    argv[2] = NULL;
    argv[3] = NULL;
    argc = argc - 2;
	
    return fuse_main(argc, argv, &xmp_oper, &data);
}
