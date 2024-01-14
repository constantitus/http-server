/*  All the functions I don't want to write twice */

#ifndef HELPERS_H
#define HELPERS_H

/*  Returns 1 if the buffer begins with string 'str'.
    Returns 0 in all other cases, including errors and 'str' being longer than
    buffer */
int buffer_begins(const char *buffer, const char *str);

/*  Returns the length of a string in bytes.
    Unlike strlen, this function does not segfault when you give it a NULL
    pointer, instead it returns zero */
int string_len(const char *str);

/*  Concatenates two strings.
    Will guarantee a NULL pointer at end of destination even if source is too
    large to fit.
    Will not cut off the copying in the middle of a UTF8 codepoint when out of
    space.
    Returns 1 if cat was successful.
    Returns 2 if source was too large.
    Returns 0 in case of error. */
int string_cat(char *destination, const char *source, int size);

/*  Look for an instance of 'to_find' in string returns index in string where
    'to_find' was found.
    Returns -1 when nothing was found */
int string_find(const char *string, const char *to_find);

/*    Returns index of first instance of a character in a string.
    Returns -1 if not found */
int string_find_char(const char *s, const char to_find);

/*  Removes an ascii char or unicode codepoint at front of string
    assumes a valid utf8 string */
void string_shift(char *str);

/*  Safer alternative to strcpy or strncpy. int 'size' is the size of the
    'destination'.
	At most 'size' - 1 bytes will be written to 'destination' plus a NULL
    character. This prevents buffer overflows.
	You will get a NULL terminated string in 'destination' (unlike strncpy).
	Will not cut off the copying in the middle of a UTF8 codepoint when out of
    space.
	Returns 1 if copy was successful.
	Returns 2 if source could not be fully copied.
	Returns 0 in case of error.

	Assume char *source is a correctly formatted UTF8 string. */
int string_copy(char *destination, const char *source, int size);

#endif
