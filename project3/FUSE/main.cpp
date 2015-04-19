#include <bson.h>
#include <mongoc.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

//Mongo relevant structs
mongoc_client_t *client;
mongoc_collection_t *collection;
mongoc_cursor_t *cursor;
bson_t *query;


//Support fields
char *client_str;
char *get_url;
char *master = "rpfs_master_db";

//CURL relevant structs
CURL *curl_handle;
static const char *headerfilename = "head.out";
FILE *headerfile;
static const char *bodyfilename = "body.out";
FILE *bodyfile;

//Start FUSE code
static int pfs_getattr(const char *path, struct stat *stbuf) {
    char *str;
    const bson_t *doc;

    //MONGODB CLIENT CODE SETUP START
    mongoc_init();
    client = mongoc_client_new(client_str);
    collection = mongoc_client_get_collection(client, "test", "test");
    query = bson_new();
    BSON_APPEND_UTF8(query, "dirents", path);
    cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    //MONGODB CLIENT CODE SETUP END


    //CURL GET CODE START
    curl_global_init(CURL_GLOBAL_ALL);
    /* init the curl session */
    curl_handle = curl_easy_init();
    /* set URL to get */
    curl_easy_setopt(curl_handle, CURLOPT_URL, get_url);
    /* no progress meter please */
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

    /* open the header file */
    headerfile = fopen(headerfilename, "wb");
    if (!headerfile) {
        curl_easy_cleanup(curl_handle);
        return -1;
    }

    /* open the body file */
    bodyfile = fopen(bodyfilename, "wb");
    if (!bodyfile) {
        curl_easy_cleanup(curl_handle);
        fclose(headerfile);
        return -1;
    }

    /* we want the headers be written to this file handle */
    curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, headerfile);
    /* we want the body be written to this file handle instead of stdout */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, bodyfile);
    /* get it! */
    curl_easy_perform(curl_handle);
    //CURL GET CODE END

    //TODO: Get size of picture
    int pic_size = 1000;

    int res = 0;
    memset(stbuf, 0, sizeof(struct stat));

    while (mongoc_cursor_next(cursor, &doc)) {
        str = bson_as_json(doc, NULL);
        if (strcmp(path, "/") == 0) {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
        } else if (strcmp(path, str) == 0) {
            stbuf->st_mode = S_IFREG | 0666;
            stbuf->st_nlink = 1;
            //TODO: store size of picture:  stbuf->st_size = strlen(str);
        } else
            res = -ENOENT;
        bson_free(str);
    }

    /* close the header file */
    fclose(headerfile);
    /* close the body file */
    fclose(bodyfile);
    /* clean up CURL object */
    curl_easy_cleanup(curl_handle);

    /* clean up Mongo objects*/
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);

    return res;
}

static int pfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    char *str;
    const bson_t *doc;

    //MONGODB CLIENT CODE SETUP START
    mongoc_init();
    client = mongoc_client_new(client_str);
    collection = mongoc_client_get_collection(client, "test", "test");
    query = bson_new(); // Leave query empty to get all results
    cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    //MONGODB CLIENT CODE SETUP END

    if (strcmp(path, "/") != 0)
        return -ENOENT;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    while (mongoc_cursor_next(cursor, &doc)) {
        str = bson_as_json(doc, NULL);
        filler(buf, str, NULL, 0); // filler(buffer, string_to_write, statbuffer, offset)
        bson_free(str);
    }

    /* clean up Mongo objects*/
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);

    return 0;
}

static int pfs_open(const char *path, struct fuse_file_info *fi) {
    if ((fi->flags & 3) != O_RDONLY)
        return -EACCES;

    return 0;
}

static int pfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    size_t len;
    size_t get_len;
    (void) fi;
    char *str;
    char *line;
    ssize_t read;
    const bson_t *doc;

    //MONGODB CLIENT CODE SETUP START
    mongoc_init();
    client = mongoc_client_new(client_str);
    collection = mongoc_client_get_collection(client, "test", "test");
    query = bson_new();
    BSON_APPEND_UTF8(query, "dirents", path);
    cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    //MONGODB CLIENT CODE SETUP END

    while (mongoc_cursor_next(cursor, &doc)) {
        str = bson_as_json(doc, NULL);

        //TODO:
        //Do something with string to get information for HTTP GET

        bson_free(str);
    }

    //CURL GET CODE START
    curl_global_init(CURL_GLOBAL_ALL);
    /* init the curl session */
    curl_handle = curl_easy_init();
    /* set URL to get */
    curl_easy_setopt(curl_handle, CURLOPT_URL, get_url);
    /* no progress meter please */
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
    /* open the header file */
    headerfile = fopen(headerfilename, "wb");
    if (!headerfile) {
        curl_easy_cleanup(curl_handle);
        return -1;
    }
    /* open the body file */
    bodyfile = fopen(bodyfilename, "wb");
    if (!bodyfile) {
        curl_easy_cleanup(curl_handle);
        fclose(headerfile);
        return -1;
    }
    /* we want the headers be written to this file handle */
    curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, headerfile);
    /* we want the body be written to this file handle instead of stdout */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, bodyfile);
    /* get it! */
    curl_easy_perform(curl_handle);
    //CURL GET CODE END

    //TODO: len = len(image_data)
    while ((read = getline(&line, &get_len, )) != -1) {
        printf("Retrieved line of length %zu :\n", read);
        printf("%s", line);
        if (offset < len) {
            if (offset + size > len)
                size = len - offset;
            memcpy(buf, line + offset, strlen(line));
        } else
            size = 0;
    }


    /* close the header file */
    fclose(headerfile);
    /* close the body file */
    fclose(bodyfile);
    /* clean up CURL object */
    curl_easy_cleanup(curl_handle);

    /* clean up Mongo objects*/
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);

    if (line)
        free(line);

    return size;
}

//CURL put method
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    int written = fwrite(ptr, size, nmemb, (FILE *) stream);
    return written;
}

//CURL get method
static size_t read_callback(void *ptr, size_t size, size_t nmemb,
                            void *stream) {
    size_t retcode;
    curl_off_t nread;

    /* in real-world cases, this would probably get this data differently
       as this fread() stuff is exactly what the library already would do
       by default internally */
    retcode = fread(ptr, size, nmemb, stream);

    nread = (curl_off_t) retcode;

    fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T
            " bytes from file\n", nread);

    return retcode;
}

int pfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int retstat = 0;
    //BBFS code -> convert to MongoDB code
/*
    log_msg("\nbb_write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
            path, buf, size, offset, fi);
    // no need to get fpath on this one, since I work from fi->fh not the path
    log_fi(fi);

    retstat = pwrite(fi->fh, buf, size, offset);
    if (retstat < 0)
        retstat = bb_error("bb_write pwrite");*/

    return retstat;
}

/** Possibly flush cached data
 *
 * BIG NOTE: This is not equivalent to fsync().  It's not a
 * request to sync dirty data.
 *
 * Flush is called on each close() of a file descriptor.  So if a
 * filesystem wants to return write errors in close() and the file
 * has cached dirty data, this is a good place to write back data
 * and return any errors.  Since many applications ignore close()
 * errors this is not always useful.
 *
 * NOTE: The flush() method may be called more than once for each
 * open().  This happens if more than one file descriptor refers
 * to an opened file due to dup(), dup2() or fork() calls.  It is
 * not possible to determine if a flush is final, so each flush
 * should be treated equally.  Multiple write-flush sequences are
 * relatively rare, so this shouldn't be a problem.
 *
 * Filesystems shouldn't assume that flush will always be called
 * after some writes, or that if will be called at all.
 *
 * Changed in version 2.2
 */
int pfs_flush(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;

    //BBFS code -> convert to MongoDB code
    /*log_msg("\nbb_flush(path=\"%s\", fi=0x%08x)\n", path, fi);
    // no need to get fpath on this one, since I work from fi->fh not the path
    log_fi(fi);*/

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

    //BBFS code
    /*log_msg("\nbb_release(path=\"%s\", fi=0x%08x)\n",
            path, fi);
    log_fi(fi);

    // We need to close the file.  Had we allocated any resources
    // (buffers etc) we'd need to free them here as well.
    retstat = close(fi->fh);*/

    return retstat;
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
    //BBFS code
    /*log_msg("\nbb_destroy(userdata=0x%08x)\n", userdata);*/
    return;
}


/**
 * Create and open a file
 *
 * If the file does not exist, first create it with the specified
 * mode, and then open it.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the mknod() and open() methods
 * will be called instead.
 *
 * Introduced in version 2.5
 */
int pfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    int retstat = 0;

    //BBFS code -> convert to MongoDB code
    /*char fpath[PATH_MAX];
    int fd;

    log_msg("\nbb_create(path=\"%s\", mode=0%03o, fi=0x%08x)\n",
            path, mode, fi);
    bb_fullpath(fpath, path);

    fd = creat(fpath, mode);
    if (fd < 0)
        retstat = bb_error("bb_create creat");

    fi->fh = fd;

    log_fi(fi);*/

    return retstat;
}

//Start FUSE code again
static struct fuse_operations pfs_oper = {
        .getattr    = pfs_getattr,
        .readdir    = pfs_readdir,
        .open        = pfs_open,
        .read        = pfs_read,
        .write      = pfs_write,

        // MIGHT NOT NEED START:
        .flush      = pfs_flush,
        .release    = pfs_release,
        .destroy    = pfs_destroy,
        // END;

        .create     = pfs_create,
        .rename     = pfs_rename,
};

/** Rename a file */
// both path and newpath are fs-relative
int pfs_rename(const char *path, const char *newpath)
{
    int retstat = 0;

    //BBFS code -> convert to MongoDB code
    /*char fpath[PATH_MAX];
    char fnewpath[PATH_MAX];

    log_msg("\nbb_rename(fpath=\"%s\", newpath=\"%s\")\n",
            path, newpath);
    bb_fullpath(fpath, path);
    bb_fullpath(fnewpath, newpath);

    retstat = rename(fpath, fnewpath);
    if (retstat < 0)
        retstat = bb_error("bb_rename rename");*/

    return retstat;
}

int main(int argc, char *argv[]) {
    mongoc_init();
    char *str;

    client = mongoc_client_new("mongodb://localhost:27017/");
    collection = mongoc_client_get_collection(client, "test", "test");
    query = bson_new();
    cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0,
                                    query, NULL, NULL);

    while (mongoc_cursor_next(cursor, &doc)) {
        str = bson_as_json(doc, NULL);
        printf("%s\n", str);
        bson_free(str);
    }

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);

    return fuse_main(argc, argv, &pfs_oper, NULL);

}