#define FUSE_USE_VERSION 26

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
#define USED_CLOCK CLOCK_PROCESS_CPUTIME_ID
#define NANOS 1000000000LL

struct timespec begin, current;
long start,elapsed,microseconds;

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

void log_message(const char* message)
{
    FILE* f;

    f = fopen("/home/ubuntu/cs-3210/project3/FUSE/log.txt", "a");
    fprintf(f, "%s\n", message);
    fclose(f);
}

//Start FUSE code
static int pfs_getattr(const char *path, struct stat *stbuf)
{
    clock_gettime(USED_CLOCK,&begin);
    FILE *dirlist_ptr;
    FILE *stats;
    char *line = NULL;
    char *token = NULL;
    char delim[2] = ",";

    int psize = 0; // picture size
    int res = 0;
    int found_file = 0;
    ssize_t len = 0;
    ssize_t read;
    
    stats = fopen("/tmp/rpfs/stats/getattr.txt", "a");
    memset(stbuf, 0, sizeof(struct stat));
    
    if (strcmp(path, "/") == 0)
    {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        found_file = 1;
    }
    else
    {
        dirlist_ptr = fopen("/tmp/rpfs/dir/dirlist.txt", "r");       // expecting this file to exist by default

        if (dirlist_ptr == NULL)
        {
            exit(EXIT_FAILURE);
        }

        while ((read = getline(&line, &len, dirlist_ptr)) != -1)
        {
            token = strtok(line, delim);
            psize = atoi(strtok(NULL, delim));

            if (strcmp(token, path) == 0)
            {
                stbuf->st_mode = S_IFREG | 0666;
                stbuf->st_nlink = 1;
                stbuf->st_size = psize;
                found_file = 1;
                break;
            }
        }

        fclose(dirlist_ptr);
    }

    if (!found_file)
    {
        res = -ENOENT;
    }

    clock_gettime(USED_CLOCK,&current);
    start = begin.tv_sec*NANOS + begin.tv_nsec;
    elapsed = current.tv_sec*NANOS + current.tv_nsec - start;
    microseconds = elapsed / 1000 + (elapsed % 1000 >= 500);
    fprintf(stats, "%lu\n", microseconds);
    fclose(stats);

    return res;
}

static int pfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{   

    clock_gettime(USED_CLOCK,&begin);
    log_message("pfs_readdir");
    FILE *dirlist_ptr;
    FILE *stats;
    char *line = NULL;
    char *token = NULL;
    char delim[2] = ",";
    ssize_t read;
    size_t len = 0;

    filler(buf, ".", NULL, 0); // filler function formats our provided strings for an ls command
    filler(buf, "..", NULL, 0);
    stats = fopen("/tmp/rpfs/stats/readdir.txt", "a");
    dirlist_ptr = fopen("/tmp/rpfs/dir/dirlist.txt", "r"); // expecting this file to exist by default

    if (dirlist_ptr == NULL)
    {
        exit(EXIT_FAILURE);
    }

    while ((read = getline(&line, &len, dirlist_ptr)) != -1) // while we are still getting line data
    {
        token = strtok(line, delim); // token = file name
        filler(buf, token + 1, NULL, 0); // hack to remove slash
    }

    clock_gettime(USED_CLOCK,&current);
    start = begin.tv_sec*NANOS + begin.tv_nsec;
    elapsed = current.tv_sec*NANOS + current.tv_nsec - start;
    microseconds = elapsed / 1000 + (elapsed % 1000 >= 500);
    fprintf(stats, "%lu\n", microseconds);
    fclose(stats);
    fclose(dirlist_ptr);

    return 0;
}

static int pfs_open(const char *path, struct fuse_file_info *fi)
{
    clock_gettime(USED_CLOCK,&begin);
    log_message("pfs_open");
    FILE *dirlist_ptr;
    FILE *read_ptr;
    FILE *stats;
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

    strcpy(py_name, py_path);       // building full path to temporary file
    strcpy(read_name, read_path);   // building full path to actual file
    dirlist_ptr = fopen("/tmp/rpfs/dir/dirlist.txt", "r"); // expecting this file to exist by default
    stats = fopen("/tmp/rpfs/open.txt", "a");

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

    //read file from tmp/rpfs/read
    not_done = 1;
    while (not_done && count < 50) // keep looping and checking for existence of file
    {
        sleep(1);
        fd_read = open(read_name, fi->flags);
        if (fd_read != -1)
        {
            not_done = 0;
            fi->fh = fd_read;
        }
        count++;
    }

    if(not_done)
    {
        fclose(dirlist_ptr);
        return -ENOENT;
    }

    clock_gettime(USED_CLOCK,&current);
    start = begin.tv_sec*NANOS + begin.tv_nsec;
    elapsed = current.tv_sec*NANOS + current.tv_nsec - start;
    microseconds = elapsed / 1000 + (elapsed % 1000 >= 500);
    fprintf(stats, "%lu\n", microseconds);
    fclose(stats);

    return 0;
}

static int pfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    clock_gettime(USED_CLOCK,&begin);
    log_message("pfs_read");
    FILE *stats;
    char *delete_read_path = "/tmp/rpfs/read/";
    char delete_read[100];
    int res;

    stats = fopen("/tmp/rpfs/stats/read.txt", "a");
    strcpy(delete_read, delete_read_path);
    strcat(delete_read, path); // building absolute path to remove the temporary read file

    (void) path;
    res = pread(fi->fh, buf, size, offset);
    if (res == -1)
    {
        res = -ENOENT;
    }

    clock_gettime(USED_CLOCK,&current);
    start = begin.tv_sec*NANOS + begin.tv_nsec;
    elapsed = current.tv_sec*NANOS + current.tv_nsec - start;
    microseconds = elapsed / 1000 + (elapsed % 1000 >= 500);
    fprintf(stats, "%lu\n", microseconds);
    fclose(stats);

    remove(delete_read); // remove temporary read file
    return res;
}

int pfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    clock_gettime(USED_CLOCK,&begin);
    FILE *stats;
    log_message("pfs_write");
    int res;

    stats = fopen("/tmp/rpfs/stats/write.txt", "a");
    (void) path;
    res = pwrite(fi->fh, buf, size, offset);

    if (res == -1)
    {
        res = -ENOENT;
    }

    clock_gettime(USED_CLOCK,&current);
    start = begin.tv_sec*NANOS + begin.tv_nsec;
    elapsed = current.tv_sec*NANOS + current.tv_nsec - start;
    microseconds = elapsed / 1000 + (elapsed % 1000 >= 500);
    fprintf(stats, "%lu\n", microseconds);
    fclose(stats);

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
    log_message("pfs_destroy");
    remove("/tmp/rpfs/dir/dirlist.txt");
    remove("/tmp/rpfs/stats/getattr.txt"); 
    remove("/tmp/rpfs/stats/open.txt");
    remove("/tmp/rpfs/stats/read.txt");
    remove("/tmp/rpfs/stats/readdir.txt");
    remove("/tmp/rpfs/stats/write.txt");
    remove("/tmp/rpfs/stats/create.txt");
    rmdir("/tmp/rpfs/pyreadpath");
    rmdir("/tmp/rpfs/read");
    rmdir("/tmp/rpfs/write");
    rmdir("/tmp/rpfs/dir");
    rmdir("/tmp/rpfs/unlink");
    rmdir("/tmp/rpfs/stats");
    rmdir("/tmp/rpfs");
    return;
}

int pfs_create(const char* path, mode_t mode, struct fuse_file_info *fi)
{
    clock_gettime(USED_CLOCK,&begin);
    log_message("pfs_create");
    FILE* dirlist;
    FILE* f;
    FILE* stats;
    char* file_path_base = "/tmp/rpfs/write";
    char* file_path;
    char *md5_create;
    char *line = NULL;
    char *token = NULL;
    char *md5_check; 
    char delim[2] = ",";
    ssize_t len = 0;
    ssize_t read;
    int psize = 0;

    file_path = malloc(strlen(file_path_base) + strlen(path));
    strcpy(file_path, file_path_base);
    strcat(file_path, path);
    md5_create = str2md5(file_path,strlen(file_path)); 
    stats = fopen("/tmp/rpfs/stats/create.txt", "a");
    dirlist = fopen("/tmp/rpfs/dir/dirlist.txt", "a");

    while ((read = getline(&line, &len, dirlist)) != -1) // while we are still getting line data
    {
        token = strtok(line, delim); // token = file name
        psize = atoi(strtok(NULL, delim));
        md5_check = strtok(NULL, delim);
        if(strcmp(md5_create,md5_check) == 0)
        {
            log_message("Duplicate hash.");
            return -ENOENT;
        }
    }

    fprintf(dirlist, "%s,%i\n", path, 0);
    fclose(dirlist);
    f = fopen(file_path, "w");
    fi->fh = (uint64_t)fileno(f);
    free(file_path);
    clock_gettime(USED_CLOCK,&current);
    start = begin.tv_sec*NANOS + begin.tv_nsec;
    elapsed = current.tv_sec*NANOS + current.tv_nsec - start;
    microseconds = elapsed / 1000 + (elapsed % 1000 >= 500);
    fprintf(stats, "%lu\n", microseconds);
    fclose(stats);

    return 0;
}

/** Remove a file */
int pfs_unlink(const char *path)
{
    log_message("pfs_unlink");
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

void *pfs_init(struct fuse_conn_info *conn)
{
    printf("ya boi, out here, mounting programs");
    return;
}

int pfs_release(const char *path, struct fuse_file_info *fi)
{
    log_message("pfs_release");
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
        .create     = pfs_create,
        .init       = pfs_init,
        .destroy    = pfs_destroy,
        .release    = pfs_release,
};

int main(int argc, char *argv[])
{
    log_message("main");
    FILE* dirlist;

    //Init tmp directories for DB communication
    mkdir("/tmp/rpfs", 0777);
    mkdir("/tmp/rpfs/pyreadpath", 0777); //path for requested files placed here by FUSE, title is file to read
    mkdir("/tmp/rpfs/read", 0777); //files to read placed here by python script
    mkdir("/tmp/rpfs/write", 0777); //files to write placed here by FUSE
    mkdir("/tmp/rpfs/dir", 0777); //files with list of file names and size placed here by python script
    mkdir("/tmp/rpfs/unlink", 0777); //files to remove placed here by FUSE, title is file to remove
    dirlist = fopen("/tmp/rpfs/dir/dirlist.txt", "w");
    fclose(dirlist);
    mkdir("/tmp/rpfs/stats", 0777);
    fclose(fopen("/tmp/rpfs/stats/getattr.txt", "w")); // create statistics files
    fclose(fopen("/tmp/rpfs/stats/open.txt", "w"));
    fclose(fopen("/tmp/rpfs/stats/read.txt", "w"));
    fclose(fopen("/tmp/rpfs/stats/readdir.txt", "w"));
    fclose(fopen("/tmp/rpfs/stats/write.txt", "w"));
    fclose(fopen("/tmp/rpfs/stats/create.txt", "w"));

    if ((argc < 2) || (argv[argc - 1][0] == '-')) // abort if there are less than 2 provided argument or if the path starts with a hyphen (breaks)
    {
        abort();
    }

    return fuse_main(argc, argv, &pfs_oper, NULL);
}
