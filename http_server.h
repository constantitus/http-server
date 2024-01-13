#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

// TODO: implement http response
enum http_response_status {
    HTTP_OK = 200,
    HTTP_NOT_FOUND = 404,
};

typedef enum {
    HTTP_GET = 1,
    HTTP_POST = 2,
} http_method;

typedef struct http_server http_server;

typedef struct {
    http_server *server;
    int fd;
    http_method method;
    char *header;
    char *query;
    char *path;
} http_request;

typedef struct {
    void (**func)(http_request *);
    char **path;
    int count;
} http_handler_data;

struct http_server {
    http_handler_data *handlers;
};


/*  http_server initializer.
    Result must be freed */
http_server *http_server_new();

/*  http_request initializer
    Result must be freed */
http_request *http_request_new(int fd, http_server *server);

/*  Frees the request */
void http_free_request(http_request *r);

/*  Set a 'handler' function for a specific 'path'.
    The 'handler' function must take a http_request * as an argument.
    Memory for the http_request * is managed by the library and fd is closed 
    automatically when the handler returns. */
int http_handle(http_server *server,
                char *pattern,
                void (*handler)(http_request *));

/*  Start the server */
int http_listen_and_serve(http_server *server, int port);





#endif
