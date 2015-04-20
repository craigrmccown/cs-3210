#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#if defined(__APPLE__)
#  define COMMON_DIGEST_FOR_OPENSSL

#  include <CommonCrypto/CommonDigest.h>

#  define SHA1 CC_SHA1
#else
#  include <openssl/md5.h>
#endif

//MD5 Algo
char *str2md5(const char *str, int length)
{
    int n;
    MD5_CTX c;
    unsigned char digest[16];
    char *out = (char *) malloc(33);
    MD5_Init(&c);
    while (length > 0)
    {
        if (length > 512)
        {
            MD5_Update(&c, str, 512);
        } else
        {
            MD5_Update(&c, str, length);
        }
        length -= 512;
        str += 512;
    }
    MD5_Final(digest, &c);
    for (n = 0; n < 16; ++n)
    {
        snprintf(&(out[n * 2]), 16 * 2, "%02x", (unsigned int) digest[n]);
    }
    return out;
}

//Start FUSE code
static int pfs_getattr(const char *path, struct stat *stbuf)
{
    FILE *dirlist_ptr;
    char *line = NULL;
    char *token = NULL;
    char delim[2] = ",";

    int psize = 0; // picture size
    int res = 0;
    size_t len = 0;
    ssize_t read;

    memset(stbuf, 0, sizeof(struct stat));
    dirlist_ptr = fopen("/tmp/rpfs/dir/dirlist.txt", "r");       // expecting this file to exist by default

    if (dirlist_ptr == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, dirlist_ptr)) != -1) // while we are still getting line data
    {
        token = strtok(line, delim);                         //token = file name
        if (strcmp(path, "/") == 0)
        {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
        } else if (strcmp(path, token) == 0)
        {
            token = strtok(NULL, delim);                    // grab token = file size, as a char
            psize = atoi(token);
            stbuf->st_mode = S_IFREG | 0666;
            stbuf->st_nlink = 1;
            stbuf->st_size = psize;
        } else
            res = -ENOENT;
    }
    return res;
}

static int pfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    FILE *dirlist_ptr;
    char *line = NULL;
    char *token = NULL;
    char delim[2] = ",";
    ssize_t read;
    size_t len = 0;

    filler(buf, ".", NULL, 0); // filler function formats our provided strings for an ls command
    filler(buf, "..", NULL, 0);

    dirlist_ptr = fopen("/tmp/rpfs/dir/dirlist.txt", "r"); // expecting this file to exist by default

    if (dirlist_ptr == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, dirlist_ptr)) != -1) // while we are still getting line data
    {
        token = strtok(line, delim); // token = file name
        filler(buf, token, NULL, 0);
    }

    if (strcmp(path, "/") != 0) // just check if path is / (error out) else we don't care what it is
        return -ENOENT;

    return 0;
}

static int pfs_open(const char *path, struct fuse_file_info *fi)
{
    FILE *dirlist_ptr;
    FILE *read_ptr;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    ssize_t read;
    char *line = NULL;
    char *name_token = NULL;
    char py_name[100];      // string for python to query db with
    char read_name[100];    // string for FUSE to grab file name for read directory
    char write_name[100];   // string for FUSE to grab file name for write directory
    char *py_path = "/tmp/rpfs/pyreadpath/";
    char *read_path = "/tmp/rpfs/read/";
    char *write_path = "/tmp/rpfs/write/";
    char line_delim[2] = ",";
    size_t len = 0;
    int not_done = 1;
    int count = 0;
    int fd_read;
    int fd_write;

    strcpy(py_name, py_path);       // building full path to temporary file
    strcpy(read_name, read_path);   // building full path to actual file
    dirlist_ptr = fopen("/tmp/rpfs/dir/dirlist", "r"); // expecting this file to exist by default

    if (strcmp(path, "/") != 0)
        return -ENOENT;

    if (dirlist_ptr == NULL)
        exit(EXIT_FAILURE);

    while (((read = getline(&line, &len, dirlist_ptr)) != -1) && not_done) // while we are still getting line data
    {
        name_token = strtok(line, line_delim);  // name_token = file name
        if (strcmp(path, name_token) == 0)
        {
            strcat(py_name, name_token);
            strcat(read_name, name_token);      // finish building read path here since we have file name
            read_ptr = fopen(py_name, "w");     // write empty file with file name we need from db to /tmp/rpfs/pyreadpath
            fclose(read_ptr);                   // just needed to create the file
            not_done = 0;                       // we done
        }
    }

    if (not_done)   // file doesn't exist, so create it
    {
        strcpy(write_name, write_path);
        strcat(write_name, path); // build path for write
        fd_write = open(write_name, O_RDWR | O_CREAT | O_TRUNC, mode);
        fi->fh = fd_write;
    } else          // file exists or we are updating
    {
        //read file from tmp/rpfs/read
        not_done = 1;
        while (not_done && count < 50) // keep looping and checking for existence of file
        {
            sleep(1);
            fd_read = open(read_name, fi->flags);
            if (fd_read != -1)
            {
                not_done = 0;
            }
            count++;
        }

        if (not_done)       // got the file for read
        {
            fi->fh = fd_read;
        } else              // something went wrong
        {
            return -ENOENT;
        }
    }

    fclose(dirlist_ptr);
    return 0;
}

static int pfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int res;
    (void) path;
    res = pread(fi->fh, buf, size, offset);
    if (res == -1)
        res = -ENOENT;

    return res;
}

int pfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int res;
    (void) path;
    res = pwrite(fi->fh, buf, size, offset);

    if (res == -1)
        res = -ENOENT;

    return res;
}

/**
 * Clean up filesystem
 *
 * Called on filesystem exit.
 *
 * Introduced in version 2.3
 */
void pfs_destroy(void *userdata)
{
    rmdir("/tmp/rpfs/pyreadpath");
    rmdir("tmp/rpfs/read");
    rmdir("tmp/rpfs/write");
    rmdir("tmp/rpfs/dir");
    rmdir("tmp/rpfs/remove");
    return;
}

/** Remove a file */
int pfs_unlink(const char *path)
{
    int retstat = 0;
    char *remove_path = "/tmp/rpfs/unlink/";
    char remove[100];
    FILE *remove_ptr;

    strcpy(remove, remove_path);
    strcat(remove, path);

    remove_ptr = fopen(remove, "w");
    fclose(remove_ptr); // just need to create the file

    return retstat;
}


/** Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.  It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 *
 * Changed in version 2.2
 */
int pfs_release(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;

    retstat = close(fi->fh);

    return retstat;
}

//Start FUSE code again
static struct fuse_operations pfs_oper = {
        .getattr    = pfs_getattr,
        .readdir    = pfs_readdir,
        .open       = pfs_open,
        .read       = pfs_read,
        .write      = pfs_write,
        .unlink     = pfs_unlink,

        // Only need if directories in tmp will be controlled by FUSE
        .destroy    = pfs_destroy,
        .release    = pfs_release,

};

int main(int argc, char *argv[])
{

    //Init tmp directories for DB communication
    mkdir("/tmp/rpfs/pyreadpath", 0777); //path for requested files placed here by FUSE, title is file to read
    mkdir("tmp/rpfs/read", 0777); //files to read placed here by python script
    mkdir("tmp/rpfs/write", 0777); //files to write placed here by FUSE
    mkdir("tmp/rpfs/dir", 0777); //files with list of file names and size placed here by python script
    mkdir("tmp/rpfs/remove", 0777); //files to remove placed here by FUSE, title is file to remove

    if ((argc < 2) || (argv[argc - 1][0] == '-')) // abort if there are less than 2 provided argument or if the path starts with a hyphen (breaks)
        abort();

    return fuse_main(argc, argv, &pfs_oper, NULL);

}