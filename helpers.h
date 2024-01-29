/*  Miscellaneous helper functions I don't want to write twice. */
#ifndef HELPERS_H
#define HELPERS_H

#include <stddef.h>

/*  Looks for an instance of c in s and returns it's index.
    Returns -1 if it wasn't found. */
int str_findchar(const char *s, const char c);

#define str_findstr(x, y) str_findstring(x, strlen(x), y, strlen(y))
/*  Looks for an instance of s2 in s1 and returns it's index.
    Returns -1 if it wasn't found. */
int str_findstring(const char *s1, size_t len1, const char *s2, size_t len2);

/*  Checks to see if s1 starts with s2 and returns 1 if it does.
    Returns 0 if it s1 doesn't start with s2 or is empty. */
int str_starts(const char *s1, const char *s2);

/*  str_split slices 'str' into all substrings separated by 'sep' and returns a
    char ** of the substrings between those separators. 'count' is the number
    of substrings returned (or the number of 'sep' occurances + 1).

    Returns NULL when 'str' or 'sep' is empty.
    Returns a char ** with a single element containing 'str' if 'sep' was not
    found.

    Resulting char ** must be freed if not NULL.
    Each char * element must be freed aswell. */
char **str_split(const char *str, const char *sep, int *count);

#endif
