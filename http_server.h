#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <sys/types.h>
#include <netinet/in.h>

typedef enum {
    HTTP_OK = 200,
    HTTP_NOT_FOUND = 404,
} http_status;

typedef struct {
    char **headers;
    char **responses;
    int headers_count;
    int resp_count;
    http_status status;
} http_response_writer;

typedef enum {
    HTTP_GET = 1,
    HTTP_POST = 2,
} http_method;

typedef struct http_server http_server;

typedef struct {
    const http_server *server;
    int *fd;
    http_method method;
    char *header;
    char *query;
    char *path;
} http_request;

typedef struct {
    void (*func)(http_request *, http_response_writer *);
    char *path;
} http_handlers;

struct http_server {
    http_handlers **handlers;
    int handlers_count;
    struct sockaddr_in *addr;
    int max_connections;
};


/*  http_server initializer. */
http_server *http_server_new(int port);

/*  Set a 'handler' function for a specific 'path'.
    The 'handler' function must take (http_request *r, http_response_writer *2)
    as arguments.
    Memory for the http_request * is managed by the library and fd is closed 
    automatically when the handler returns.
    Returns 0 if successful. 
    Returns -1 in case of error. */
int http_handle(http_server *server,
                char *pattern,
                void (*handler)(http_request *, http_response_writer *));

/*  Listens on the TCP network address server->addr and port server->port and
    then handles the requests on incoming connections */
int http_listen_and_serve(const http_server *server);

/*  Returns the bounary from the Content-Type: multipart/form-data header.
    Returns NULL if header was not found.
    The returned char * must be freed if not NULL. */
char *http_multipart_get_boundary(http_request *r);

/*  Reads the header 'name' from r->headers.
    Returns NULL if header was not found.
    The returned char * must be freed if not NULL. */
char *http_header_get(http_request *r, const char *name);


/*  Sets the status of the response header */
void http_set_status(http_response_writer *w, http_status status);

// TODO: Add more fields
/*  Calls http_set_header with the "Set-Cookie" header name and the cookie
    data as value.
    Returns 0 if successful.
    Returns -1 in case of error. */
int http_set_cookie(http_response_writer *w,
                    const char *name,
                    const char *value,
                    ...);

/*  Adds a header to the provided http_response_writer's headers.
    Returns 0 if successful.
    Returns -1 in case of error. */
int http_set_header(http_response_writer *w,
                    const char *name,
                    const char *value);
                    
/*  Writes to the client fd after the header has been sent.
    This is used to write the html response or to send data to the client.
    Returns 0 if successful.
    Returns -1 in case of error. */
int http_write(http_response_writer *w, const char *buf, size_t len);



#endif
