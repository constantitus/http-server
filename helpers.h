/*  All the functions I don't want to write twice */

#ifndef HELPERS_H
#define HELPERS_H

/*  Returns 1 if the buffer begins with string 'str'.
    Returns 0 in all other cases, including errors and 'str' being longer than
    buffer */
int buffer_begins(char *buffer, char *str);

/*  Returns the length of a string in bytes.
    Unlike strlen, this function does not segfault when you give it a NULL
    pointer, instead it returns zero */
int string_len(char *str);

/*  Concatenates two strings.
    Will guarantee a NULL pointer at end of destination even if source is too
    large to fit.
    Will not cut off the copying in the middle of a UTF8 codepoint when out of
    space.
    Returns 1 if cat was successful.
    Returns 2 if source was too large.
    Returns 0 in case of error. */
int string_cat(char *destination, char *source, int size);

/*  Look for an instance of 'to_find' in string returns index in string where
    'to_find' was found.
    Returns -1 when nothing was found */
int string_find(char *string, char *to_find);

/*  Same as tek_str_find_str, but starts from given index */
int string_find_from(char *string, char *to_find, int from);

/*    Returns index of first instance of a character in a string.
    Returns -1 if not found */
int string_find_char(char *s, char to_find);

/*  Same as tek_str_find_char, but starts from given index 'form'. */
int string_find_char_from(char *s, char to_find, int from);

/*  Removes an ascii char or unicode codepoint at front of string
    assumes a valid utf8 string */
void string_shift(char *str);

/*  Splits a string where char 'c' occurs.
    'num' will give you the number of times char 'c' occured in the string.
    'num' is also index of last string in returned char**.

    The string str will be modified so make sure to make a copy if needed

    Note that some character pointers may be NULL pointers if 2 or more
    characters 'c' are right next to each other

    You must free the result. */
char **string_split(char *str, char c, int *num);

#endif
