#ifndef HTTP_H
#define HTTP_H

typedef char *http_field;

#define HTTP_GET    1
#define HTTP_POST   2

#define HTTP_HEADER_OK "HTTP/1.1 200 OK\r\n" \
    "Content-Type: text/html\r\n\r\n"

/*  Returns the HTTP_GET, HTTP_POST or -1 in case of error e.g. the buffer is
    empty. */
int http_method(char *buffer);

http_field http_path(char *buffer);

/*  Takes the result of http_path() and returns the wanted query string
    or NULL if not found.
    You must free the result if result is non-NULL. */
char *http_query_from_path(http_field path, char *str);

/*  Returns the wanted query string's value from buffer or NULL if not found.
    You must free the result if result is non-NULL. */
char *http_query(char *buffer, char *str);

/*  Takes the result of http_path() and returns all query strings in the
    form of "key=value". */
char **http_query_strings(http_field path);

#define http_host(buffer) \
    http_parse_field(buffer, "\r\nHost: ", 0)

#define http_cookies(buffer) \
    http_parse_field(buffer, "\r\nCookie: ", 0)

#define http_user_agent(buffer) \
    http_parse_field(buffer, "\r\nUser-Agent: ", 0)

#define http_connection(buffer) \
    http_parse_field(buffer, "\r\nConnection: ", 0)

#define http_content_type(buffer) \
    http_parse_field(buffer, "\r\nContent-Type: ", 0)

#define http_content_length(buffer) \
    http_parse_field(buffer, "\r\nContent-Length: ", 0)

/*  From 'buffer', returns a http_field from 'start' and ending in "\r\n" - 
    end. Result is exclusive.
    'end' can be the length of " HTTP/1.1" or " HTTP/1.0".
    If start is not found, returns NULL.
    You must free the result if result is non-NULL. */
http_field http_parse_field(char *buffer, char *start, int end);

#endif
