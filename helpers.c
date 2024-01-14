#include <stdint.h>


int buffer_begins(const char *buffer, const char *str) {
    if (!buffer)
        return 0;
    if (!str)
        return 0;

    while (*str && *buffer == *str) {
        str += 1;
        buffer += 1;
    }

    if (*str) {
        return 0;
    }

    return 1;
}

int string_len(const char *str) {
    if (!str)
        return 0;
    if (!*str)
        return 0;

    int len = 0;

#if __x86_64__
    int64_t *str_i = (int64_t *)str;
    int64_t addr = (int64_t)str_i;

    // 64 bit computer
    // ensure str is on 8-byte boundary before using speed-up trick
    while (addr & 0x0000000000000007 && *str) {
        len += 1;
        str += 1;
        addr = (int64_t)str;
    }

    if (!*str) {
        return len;
    }

    // check for NULL characters, 8 bytes at a time
    // https://graphics.stanford.edu/~seander/bithacks.html#ZeroInWord
    str_i = (int64_t *)str;
    while (!((*str_i - 0x0101010101010101) & ~(*str_i) & 0x8080808080808080)) {
        len += 8;
        str_i += 1;
    }

    str = (char *)str_i;
    while (*str) {
        len += 1;
        str += 1;
    }

#else
    int32_t *str32_i = str;
    int32_t addr32 = (int32_t)str32_i;

    // 32 bit computer
    // ensure str is on 4-byte boundary before using speed-up trick
    while (addr32 & 0x00000003 && *str) {
        len += 1;
        str += 1;
        addr32 = (int32_t)str;
    }

    if (!*str) {
        return len;
    }

    // check for NULL characters, 4 bytes at a time
    // https://graphics.stanford.edu/~seander/bithacks.html#ZeroInWord
    str32_i = (int32_t *)str;
    while (!((*str32_i - 0x01010101) & ~(*str32_i) & 0x80808080)) {
        len += 4;
        str32_i += 1;
    }

    str = (char *)str32_i;
    while (*str) {
        len += 1;
        str += 1;
    }
#endif

    return len;
}

int string_cat(char *destination, const char *source, int size) {
    if (!destination)
        return 0;
    if (!source)
        return 0;
    if (!*source)
        return 1;
    if (size < 2)
        return 0;

    int len = string_len(destination);
    if (len > size)
        return 0;
    if (len == size && !*source)
        return 1;

    destination += len;
    size -= 1;

    while (*source && len < size) {
        *destination = *source;
        len += 1;
        destination += 1;
        source += 1;
    }

    // we don't want to cut off the copying in the middle of a UTF8 codepoint
    // firstly check whether the next byte of source is either not present or
    // the start of a codepoint
    if (*source && (*source & 128) &&
        !(*source & 64)) {
        destination -= 1;
        // this while loop goes back while we have the 2nd, 3rd or 4th byte of
        // a UTF8 codepoint
        while ((*destination & 128) &&
               !(*destination & 64)) {
            destination -= 1;
        }
        // this goes back from the head of a UTF8 codepoint
        if ((*destination & 128) &&
            (*destination & 64)) {
            *destination = 0;
        } else {
            // should never happen, this would be invalid UTF8
            *destination = 0;
            return 0;
        }
    } else {
        *destination = 0;
    }

    if (*source)
        return 2;
    return 1;
}

int string_find(const char *string, const char *to_find) {
    if (!string || !to_find)
        return -1;

    int len = string_len(string);
    int len2 = string_len(to_find);

    if (!len || !len2)
        return -1;
    if (len2 > len)
        return -1;

    int index = 0;
    int i = 0;
    int j = 0;

    // I don't need to check the full length of string
    // only as far as there is still enough space for it to contain to_find
    int len_loop = len - len2 + 1;
    while (index < len_loop) {
        while (index < len_loop && string[index] != to_find[0]) {
            index += 1;
        }
        if (string[index] == to_find[0]) {
            i = index;
            j = 0;
            while (i < len && j < len2 && string[i] == to_find[j]) {
                i += 1;
                j += 1;
            }
            if (j == len2) {
                return index;
            }
        }
        index += 1;
    }

    return -1;
}

int string_find_char(const char *s, char to_find) {
    if (!s)
        return -1;

    int i = 0;
    while (s[i] && to_find != s[i]) {
        i += 1;
    }

    if (s[i]) {
        return i;
    } else {
        return -1;
    }
}

void string_shift(char *str) {
    if (!str)
        return;

    int len = string_len(str);
    int i = 1;
    int j = 1;

    if ((str[i - j] & 128) &&
        (str[i - j] & 64)) {
        j += 1;
        i += 1;
        while ((str[i] & 128) &&
               !(str[i] & 64)) {
            j += 1;
            i += 1;
        }
    }

    while (i < len) {
        str[i - j] = str[i];
        i += 1;
    }
    str[i - j] = '\0';
}

int string_copy(char *destination, const char *source, int size) {
	if (!destination)
		return 0;
	if (!source) {
		*destination = 0;
		return 1;
	}
	if (size <= 0)
		return 0;

	size -= 1;

	int i = 0;
	while (*source && i < size) {
		destination[i] = *source;
		source += 1;
		i += 1;
	}

	// we don't want to cut off the copying in the middle of a UTF8 codepoint
	// firstly check whether the next byte of source is either not present or
    // the start of a codepoint
	if (*source && (*source & 128) &&
		!(*source & 64)) {
		i -= 1;
		// this while loop goes back while we have the 2nd, 3rd or 4th byte of
        // a UTF8 codepoint
		while ((destination[i] & 128) &&
			   !(destination[i] & 64)) {
			i -= 1;
		}
		// this goes back from the head of a UTF8 codepoint
		if ((destination[i] & 128) &&
			(destination[i] & 64)) {
			destination[i] = 0;
		} else {
			// should never happen, this would be invalid UTF8
			destination[i] = 0;
			return 0;
		}
	}

	destination[i] = '\0';

	if (*source) {
		return 2;
	} else {
		return 1;
	}
}

