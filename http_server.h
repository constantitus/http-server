#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

// #ifdef HTTP_VERBOSE_ERRORS

#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>

typedef struct {
    char *status;
    char *content_type;
    char **headers;
    char **responses;
    int header_cnt;
    int resp_cnt;
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

/*  http_serve_mux is the HTTP request multiplexer.
    It matches the URL of each incoming request against a list of registered
    patterns and calls the handler function for the pattern that matches the
    URL (f it doesn't match the exact path, it defaults to "/"). */
typedef struct {
    pthread_mutex_t mu;

    // Handlers
    void (**funcs)(http_request *, http_response_writer *);
    char **paths;
    int count;
} http_serve_mux;

struct http_server {
    http_serve_mux *handler;
    struct sockaddr_in *addr;
    int max_connections;
};



/*  Allocates and returns a new http_serve_mux. */
http_serve_mux *http_serve_mux_new();

/*  Allocates and returns a new http_server on the specified port. */
http_server *http_server_new(http_serve_mux *handler, int port);

/*  Set a 'handler' function for a specific 'path'.
    The http_request * and http_response_writer * are freed automatically when
    the handler function returns.
    Returns 0 if successful. 
    Returns -1 in case of error. */
int http_handle(http_serve_mux *mux,
                char *pattern,
                void (*handler)(http_request *, http_response_writer *));

/*  Listens on the TCP network address server->addr and then handles the
    requests on incoming connections. */
int http_listen_and_serve(const http_server *server);



/*  Returns the bounary from the Content-Type: multipart/form-data header.
    Returns NULL if header was not found.
    The returned char * must be freed if not NULL. */
char *http_multipart_get_boundary(http_request *r);

/*  Reads the header 'name' from r->headers.
    Returns NULL if header was not found.
    The returned char * must be freed if not NULL. */
char *http_get_header(http_request *r, const char *name);


/*  Sets the status of the response header. The default is 200 OK.
    "HTTP 1.1 " and "\r\n" are added automatically. */
int http_set_status(http_response_writer *w, char *status);

/*  Sets the Content-Type header. The default is text/html; charset=utf-8.
    The header name and \r\n are added automatically. */
void http_set_content_type(http_response_writer *w, const char *content_type);

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
