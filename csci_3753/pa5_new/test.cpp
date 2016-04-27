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

int main(){
	int res;
	char full_path = "testfile_encrypted.txt";//[MAX_FILEPATH_LENGTH];

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
	do_crypt(original, temp, 0, "swordfish");

    if(fclose(original)){
        perror("original close error\n");
    }
    if(fclose(temp)){
		perror("temp close error\n");
    }

	//  Reopen files in reverse
    original = fopen(full_path, "w");
    if(!original){
		perror("file open error\n");
		return 0;
    }

    temp = fopen("/tmp/test", "rb");
    if(!temp){
		perror("temp open error\n");
		return 0;
    }

	//  Copy temp back into original
	do_crypt(temp, original, -1, "swordfish");

    if(fclose(original)){
        perror("original close error\n");
    }
    if(fclose(temp)){
		perror("temp close error\n");
    }


	return 0;
}
