#include <stdio.h>
#include <stdlib.h>
#include "http.h"
#include "helpers.h"

int http_method(char *buffer) {
    if (buffer_begins(buffer, "POST")) {
        return HTTP_POST;
    }
    if (buffer_begins(buffer, "GET")) {
        return HTTP_GET;
    }
    return -1;
}

http_field http_path(char *buffer){
    int method = http_method(buffer);
    switch (method) {
        case HTTP_GET:
            return http_parse_field(buffer, "GET ", 9);
            break;
        case HTTP_POST:
            return http_parse_field(buffer, "POST ", 9);
            break;
    }
    return NULL;
}

http_field http_parse_field(char *buf, char *start, int end) {
    int start_i = string_find(buf, start);
    if (start_i < 0)
        return NULL;
    start_i += string_len(start);
    int end_i = string_find_from(buf, "\r\n", start_i) - end;

    http_field field = (char *)malloc((end_i - start_i) * sizeof(char));
    for (int i = 0; i < (end_i - start_i); i++) {
        field[i] = buf[start_i + i];
    }

    return field;
}

void process_query_string(char *str) {
    // replace + with space
    char *tmp = str;
    int n = string_find_char(tmp, '+');
    while (n != -1) {
        tmp += n;
        *tmp = ' ';
        n = string_find_char(tmp, '+');
    }

    int i = 0;
    tmp = str;
    while (str[i]) {
        while (str[i] && str[i] != '%') {
            i += 1;
        }
        if (str[i] == '%') {
            tmp = &(str[i]);
            int hex = 0;
            i += 1;
            int j = 1;
            while (j >= 0) {
                if (str[i] >= 'A' && str[i] <= 'F') {
                    hex += (str[i] - 'A') + 10;
                }
                if (str[i] >= '0' && str[i] <= '9') {
                    hex += (str[i] - '0');
                }
                if (j) {
                    hex *= 16;
                }
                j -= 1;
                i += 1;
            }
            tmp[0] = hex;
            tmp += 1;
            string_shift(tmp);
            string_shift(tmp);
            i -= 2;
        }
    }
}

char *http_query_from_path(http_field path, char *str) {
    int query_mark = string_find_char(path, '?');
    if (query_mark < 0)
        return NULL;
    int key_start = string_find_from(path, str, query_mark);
    if (key_start < 0)
        return NULL;
    int val_start = string_find_char_from(path, '=', key_start) + 1;
    if (val_start < 0) // should never happen under normal circumstances
        return NULL;
    int val_end = string_find_char_from(path, '&', val_start);
    if (val_end < 0)
        val_end = string_len(path) + 1;
    if (val_end <= val_start)
        return NULL;

    char *value = (char *)malloc((val_end - val_start) * sizeof(char));
    for (int i = 0; i < (val_end - val_start); i++) {
        value[i] = path[val_start + i];
    }
    // process_query_string(value);

    return value;
}

char *http_query(char *buffer, char *str) {
    http_field path = http_path(buffer);
    if (!path)
        return NULL;
    char *query =  http_query_from_path(path, str);
    free(path);
    if (query < 0)
        return NULL;
    return query;
}

char **http_query_strings(http_field path) {
    int query_start = string_find_char(path, '?');
    if (query_start < 0)
        return NULL;
    int query_len = string_len(path) - query_start;

    char *query = (char *)malloc(query_len * sizeof(char));
    for (int i = 0; i < (query_len); i++) {
        query[i] = path[i + query_start];
    }

    if (string_len(query) > 1024) {
        return NULL;
    }

    int i = 0;
    int num = 0;
    char **qss = string_split(query, '&', &num);

    while (i <= num) {
        process_query_string(qss[i]);
        i += 1;
    }

    return qss;
}
