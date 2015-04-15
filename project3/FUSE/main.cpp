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

//Start FUSE code again
static struct fuse_operations pfs_oper = {
        .getattr    = pfs_getattr,
        .readdir    = pfs_readdir,
        .open        = pfs_open,
        .read        = pfs_read,
        .write      = pfs_write,
        .flush      = pfs_flush,
        .release    = pfs_release,
        .destroy    = pfs_destroy,
        .create     = pfs_create,
        .rename     = pfs_rename,
        .chmod      = pfs_chmod,
        .chown      = pfs_chown,
};

int main(int argc, char *argv[]) {
    mongoc_init();

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