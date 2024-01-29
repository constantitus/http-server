#include <stdlib.h>
#include <string.h>

#include "helpers.h"

int str_findchar(const char *s, const char c) {
    if (!*s)
        return -1;

    int idx = 0;
    while (s[idx] && c != s[idx])
        idx++;

    if (s[idx])
        return idx;
    return -1;
}

int str_findstring(const char *s1, size_t len1, const char *s2, size_t len2) {
    if (len1 < len2)
        return -1;

    int idx = 0;
    for (;idx < len1 - len2 + 1; idx++) {
        if (s1[idx] != s2[0])
            continue;
        else {
            int i = 0;
            while (idx < len1 && s1[idx + i] == s2[i])
                i++;
            if (i == len2)
                return idx;
        }
    }
    return -1;
}

int str_starts(const char *s1, const char *s2) {
    if (!*s1)
        return 0;

    while (*s2 && *s1 == *s2) {
        s1++;
        s2++;
    }

    if (*s2)
        return 0;
    return 1;
}

char **str_split(const char *str, const char *sep, int *count) {
    if (!*str || !*sep)
        return NULL;

    int res_size = 32;
    char **res = (char **)malloc(res_size * sizeof(char *));
    int len_sep = strlen(sep);
    int len_str = strlen(str);
    int idx, found = 0;

    _Bool last = 0;
    for (idx = 0; !last; idx++) {
        found = str_findstring(str, len_str, sep, len_sep);
        if (found < 0) {
            found = strlen(str);
            last = 1; // Last itteration
        }

        if (idx >= res_size) {
            res_size += 16;
            res = (char **)realloc(res, res_size * sizeof(char *));
        }

        res[idx] = (char *)malloc(found * sizeof(char));

        memmove(res[idx], str, found * sizeof(char));
        // int i = 0;
        // for (; i < found; i++)
        //     res[idx][i] = str[i];
        res[idx][found - 1] = '\0';

        str += found + len_sep;
    }

    *count = idx;

    return res;
}
