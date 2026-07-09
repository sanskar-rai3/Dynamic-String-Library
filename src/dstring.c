#include "dstring.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* Static Datas */
static const char *const g_dstr_error_strings[] = {
	[DSTR_SUCCESS]             = "",
	[DSTR_ERROR_ALLOC_FAILED]  = "Memory allocation or reallocation failed",
	[DSTR_ERROR_OUT_OF_BOUNDS] = "Index or position out of bounds",
	[DSTR_ERROR_NULL_ARGUMENT] = "Null pointer passed as a required argument",
	[DSTR_ERROR_INVALID_SIZE]  = "Requested size or capacity change is invalid",
	[DSTR_ERROR_NOT_FOUND]     = "Substring or character target not found"
};

static DStringError g_dstr_last_error = DSTR_SUCCESS;

/* Error Handling */

/* Sets the internal global error code if the provided code is valid. */
void dstr_set_error_code(DStringError code) {
	if (code >= 0 && code < DSTR_ERROR_COUNT) {
		g_dstr_last_error = code;
	}
}

/* Returns the most recently recorded global error code. */
DStringError dstr_get_error_code(void) {
	return g_dstr_last_error;
}

/* Returns the descriptive text message corresponding to the current global error code. */
const char *dstr_get_error_string(void) {
	return g_dstr_error_strings[g_dstr_last_error];
}

/* Resets the global error status back to DSTR_SUCCESS. */
void dstr_clear_error_code(void) {
	g_dstr_last_error = DSTR_SUCCESS;
}

/* Construction / Destruction */

/* Initializes an empty dynamic string with a default capacity of 8 bytes. */
DString dstr_create(void) {
	size_t len      = 0;
	size_t capacity = 8;

	char *buffer = malloc(capacity);
	if (!buffer) {
		dstr_set_error_code(DSTR_ERROR_ALLOC_FAILED);
		return (DString){0};
	}

	buffer[len] = '\0';
	dstr_set_error_code(DSTR_SUCCESS);

	return (DString) {
		.buffer   = buffer,
		.len      = len,
		.capacity = capacity
	};
}

/* Creates a dynamic string by copying contents from a null-terminated C-string. */
DString dstr_from_cstr(const char *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return (DString){0};
	}

	size_t len   = strlen(str);
	size_t capacity = 8;
	while (capacity < len + 1) {
		capacity *= 2;
	}

	char *buffer = malloc(capacity); 
	if (!buffer) {
		dstr_set_error_code(DSTR_ERROR_ALLOC_FAILED);
		return (DString){0};
	}

	memcpy(buffer, str, len + 1);
	dstr_set_error_code(DSTR_SUCCESS);

	return (DString){
		.buffer   = buffer,
		.len      = len,
		.capacity = capacity
	};
}

/* Creates a dynamic string by copying a specified number of bytes from a raw buffer. */
DString dstr_from_buffer(const char *buffer, size_t len) {
	if (!buffer) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return (DString){0};
	}

	size_t capacity = 8;
	while (capacity < len + 1) {
		capacity *= 2;
	}

	char *new_buffer = malloc(capacity);
	if (!new_buffer) {
		dstr_set_error_code(DSTR_ERROR_ALLOC_FAILED);
		return (DString){0};
	}

	memcpy(new_buffer, buffer, len);
	new_buffer[len] = '\0';
	dstr_set_error_code(DSTR_SUCCESS);

	return (DString) {
		.buffer   = new_buffer,
		.len      = len, 
		.capacity = capacity
	};
}

/* Frees the allocated memory of the dynamic string and resets its properties to zero. */
void dstr_destroy(DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}

	free(str->buffer);
	str->buffer   = NULL;
	str->len      = 0;
	str->capacity = 0;
	dstr_set_error_code(DSTR_SUCCESS);
}

/* Capacity */

/* Returns the current length (number of characters) of the dynamic string. */
size_t dstr_len(const DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return 0;
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return str->len;
}

/* Returns the total allocated memory capacity of the dynamic string buffer. */
size_t dstr_capacity(const DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return 0;
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return str->capacity;
}

/* Checks if the string length is zero. */
bool dstr_empty(const DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return true;
	}
	
	dstr_set_error_code(DSTR_SUCCESS);
	return str->len == 0;
}

/* Resets the string length to zero and null-terminates the start without freeing memory. */
void dstr_clear(DString *str) {
	if (!str || !str->buffer) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}

	str->len       = 0;
	str->buffer[0] = '\0';
	dstr_set_error_code(DSTR_SUCCESS);
}

/* Expands the buffer capacity to at least the requested size if it is greater than current capacity. */
void dstr_reserve(DString *str, size_t capacity) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}
	
	if (capacity <= str->capacity) {
		dstr_set_error_code(DSTR_SUCCESS);
		return;
	}

	char *new_buf = realloc(str->buffer, capacity);
	if (!new_buf) {
		dstr_set_error_code(DSTR_ERROR_ALLOC_FAILED);
		return;
	}

	str->buffer = new_buf;
	str->capacity = capacity;
	dstr_set_error_code(DSTR_SUCCESS);
}

/* Resizes the string to a new length, padding with zeroes if it grows, and updates null termination. */
void dstr_resize(DString *str, size_t new_len) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}

	if (new_len + 1 > str->capacity) {
		size_t new_cap = str->capacity == 0 ? 8 : str->capacity;
		while (new_cap < new_len + 1) {
			new_cap *= 2;
		}
		dstr_reserve(str, new_cap);
		if (g_dstr_last_error == DSTR_ERROR_ALLOC_FAILED) return;
	}

	if (new_len > str->len) {
		memset(str->buffer + str->len, 0, new_len - str->len);
	}

	str->len           = new_len;
	str->buffer[new_len] = '\0';
	dstr_set_error_code(DSTR_SUCCESS);
}

/* Shrinks the memory capacity down to exactly fit the current length plus the null terminator. */
void dstr_shrink_to_fit(DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}

	size_t req = str->len + 1;
	char *new_buf = realloc(str->buffer, req);
	if (!new_buf) {
		dstr_set_error_code(DSTR_ERROR_ALLOC_FAILED);
		return;
	}

	str->buffer = new_buf;
	str->capacity = req;
	dstr_set_error_code(DSTR_SUCCESS);
}

/* Element Access */

/* Returns the character at the specified index with boundary checks. */
char dstr_at(const DString *str, size_t index) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return '\0';
	}

	if (index >= str->len) {
		dstr_set_error_code(DSTR_ERROR_OUT_OF_BOUNDS);
		return '\0';
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return str->buffer[index];
}

/* Returns the first character of the string. */
char dstr_front(const DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return '\0';
	}

	if (str->len == 0) {
		dstr_set_error_code(DSTR_ERROR_OUT_OF_BOUNDS);
		return '\0';
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return str->buffer[0];
}

/* Returns the last character of the string. */
char dstr_back(const DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return '\0';
	}

	if (str->len == 0) {
		dstr_set_error_code(DSTR_ERROR_OUT_OF_BOUNDS);
		return '\0';
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return str->buffer[str->len - 1];
}

/* Returns a mutable raw pointer to the underlying character buffer. */
char *dstr_data(DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return NULL;
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return str->buffer;
}

/* Returns a read-only C-string pointer to the underlying buffer. */
const char *dstr_cstr(const DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return NULL;
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return str->buffer;
}

/* Assignment */

/* Overwrites the contents of the destination string with a copy of the source string. */
void dstr_copy(DString *dest, const DString *src) {
	if (!dest || !src) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}

	while (dest->capacity < src->len + 1) {
		size_t next_cap = dest->capacity == 0 ? 8 : dest->capacity * 2;
		char *new_buf = realloc(dest->buffer, next_cap);
		if (!new_buf) {
			dstr_set_error_code(DSTR_ERROR_ALLOC_FAILED);
			return;
		}
		dest->buffer = new_buf;
		dest->capacity = next_cap;
	}

	dest->len = src->len;
	memcpy(dest->buffer, src->buffer, dest->len + 1);
	dstr_set_error_code(DSTR_SUCCESS);
}

/* Overwrites the contents of the destination string with a copy of a null-terminated C-string. */
void dstr_copy_cstr(DString *dest, const char *src) {
	if (!dest || !src) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}
	size_t len = strlen(src);

	while (dest->capacity < len + 1) {
		size_t next_cap = dest->capacity == 0 ? 8 : dest->capacity * 2;
		char *new_buf = realloc(dest->buffer, next_cap);
		if (!new_buf) {
			dstr_set_error_code(DSTR_ERROR_ALLOC_FAILED);
			return;
		}
		dest->buffer = new_buf;
		dest->capacity = next_cap;
	}

	memcpy(dest->buffer, src, len + 1);
	dest->len = len;
	dstr_set_error_code(DSTR_SUCCESS);
}

/* Swaps the internal buffer pointers, lengths, and capacities of two strings. */
void dstr_swap(DString *a, DString *b) {
	if (!a || !b) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}
	
	DString temp = *a;
	*a           = *b;
	*b           = temp;
	dstr_set_error_code(DSTR_SUCCESS);
}

/* Comparison */

/* Compares two dynamic strings lexicographically. Returns <0 if a<b, 0 if equal, >0 if a>b. */
int dstr_cmp(const DString *a, const DString *b) {
	if (!a || !b) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return 0;
	}

	size_t min = (a->len < b->len) ? a->len : b->len;

	for (size_t i = 0; i < min; i++) {
		if (a->buffer[i] < b->buffer[i]) return -1;
		if (a->buffer[i] > b->buffer[i]) return  1;
	}

	if (a->len < b->len) return -1;
	if (a->len > b->len) return  1;

	dstr_set_error_code(DSTR_SUCCESS);
	return 0;
}

/* Compares a dynamic string with a C-string lexicographically. */
int dstr_cmp_cstr(const DString *a, const char *b) {
	if (!a || !b) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return 0;
	}
	
	size_t b_len = strlen(b);
	size_t min = (a->len < b_len) ? a->len : b_len;

	for (size_t i = 0; i < min; i++) {
		if (a->buffer[i] < b[i]) return -1;
		if (a->buffer[i] > b[i]) return  1;
	}

	if (a->len < b_len) return -1;
	if (a->len > b_len) return  1;

	dstr_set_error_code(DSTR_SUCCESS);
	return 0;
}

/* Checks if two dynamic strings are identical in length and content. */
bool dstr_equal(const DString *a, const DString *b) {
	if (!a || !b) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return false;
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return a->len == b->len && memcmp(a->buffer, b->buffer, a->len) == 0;
}

/* Checks if a dynamic string is identical in content to a C-string. */
bool dstr_equal_cstr(const DString *a, const char *b) {
	if (!a || !b) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return false;
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return a->len == strlen(b) && memcmp(a->buffer, b, a->len) == 0;
}

/* Append */

/* Appends the contents of the source dynamic string to the end of the destination string. */
void dstr_append(DString *dest, const DString *src) {
	if (!dest || !src) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}

	size_t req = dest->len + src->len + 1;
	if (req > dest->capacity) {
		size_t new_cap = dest->capacity == 0 ? 8 : dest->capacity;
		while (new_cap < req) {
			new_cap *= 2;
		}
		dstr_reserve(dest, new_cap);
		if (g_dstr_last_error == DSTR_ERROR_ALLOC_FAILED) return;
	}

	memcpy(dest->buffer + dest->len, src->buffer, src->len + 1);
	dest->len += src->len;
	dstr_set_error_code(DSTR_SUCCESS);
}

/* Appends a null-terminated C-string to the end of the destination dynamic string. */
void dstr_append_cstr(DString *dest, const char *src) {
	if (!dest || !src) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}

	size_t src_len = strlen(src);
	size_t req = dest->len + src_len + 1;
	if (req > dest->capacity) {
		size_t new_cap = dest->capacity == 0 ? 8 : dest->capacity;
		while (new_cap < req) {
			new_cap *= 2;
		}
		dstr_reserve(dest, new_cap);
		if (g_dstr_last_error == DSTR_ERROR_ALLOC_FAILED) return;
	}

	memcpy(dest->buffer + dest->len, src, src_len + 1);
	dest->len += src_len;
	dstr_set_error_code(DSTR_SUCCESS);
}

/* Appends a single character to the end of the string. */
void dstr_push_back(DString *str, char ch) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}

	if (str->len + 2 > str->capacity) {
		size_t new_cap = str->capacity == 0 ? 8 : str->capacity * 2;
		dstr_reserve(str, new_cap);
		if (g_dstr_last_error == DSTR_ERROR_ALLOC_FAILED) return;
	}

	str->buffer[str->len] = ch;
	str->len++;
	str->buffer[str->len] = '\0';
	dstr_set_error_code(DSTR_SUCCESS);
}

/* Insert / Erase */

/* Inserts a source dynamic string into the target string at the specified position. */
void dstr_insert(DString *str, size_t pos, const DString *src) {
	if (!str || !src) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}
	
	if (pos > str->len) pos = str->len;

	size_t req = str->len + src->len + 1;
	if (req > str->capacity) {
		size_t new_cap = str->capacity == 0 ? 8 : str->capacity;
		while (new_cap < req) {
			new_cap *= 2;
		}
		dstr_reserve(str, new_cap);
		if (g_dstr_last_error == DSTR_ERROR_ALLOC_FAILED) return;
	}

	memmove(str->buffer + pos + src->len, str->buffer + pos, str->len - pos + 1);
	memcpy(str->buffer + pos, src->buffer, src->len);
	str->len += src->len;
	dstr_set_error_code(DSTR_SUCCESS);
}

/* Inserts a C-string into the dynamic string at the specified position. */
void dstr_insert_cstr(DString *str, size_t pos, const char *src) {
	if (!str || !src) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}
	
	if (pos > str->len) pos = str->len;

	size_t src_len = strlen(src);
	size_t req = str->len + src_len + 1;
	if (req > str->capacity) {
		size_t new_cap = str->capacity == 0 ? 8 : str->capacity;
		while (new_cap < req) {
			new_cap *= 2;
		}
		dstr_reserve(str, new_cap);
		if (g_dstr_last_error == DSTR_ERROR_ALLOC_FAILED) return;
	}

	memmove(str->buffer + pos + src_len, str->buffer + pos, str->len - pos + 1);
	memcpy(str->buffer + pos, src, src_len);
	str->len += src_len;
	dstr_set_error_code(DSTR_SUCCESS);
}

/* Inserts a single character into the dynamic string at the specified position. */
void dstr_insert_char(DString *str, size_t pos, char ch) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}

	if (pos > str->len) pos = str->len;

	if (str->len + 2 > str->capacity) {
		size_t new_cap = str->capacity == 0 ? 8 : str->capacity * 2;
		dstr_reserve(str, new_cap);
		if (g_dstr_last_error == DSTR_ERROR_ALLOC_FAILED) return;
	}

	memmove(str->buffer + pos + 1, str->buffer + pos, str->len - pos + 1);
	str->buffer[pos] = ch;
	str->len++;
	dstr_set_error_code(DSTR_SUCCESS);
}

/* Erases a segment of characters from the string starting at a given position and length. */
void dstr_erase(DString *str, size_t pos, size_t len) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}

	if (pos >= str->len) {
		dstr_set_error_code(DSTR_ERROR_OUT_OF_BOUNDS);
		return;
	}

	if (pos + len > str->len) len = str->len - pos;
	memmove(str->buffer + pos, str->buffer + pos + len, str->len - pos - len + 1);
	str->len -= len;
	dstr_set_error_code(DSTR_SUCCESS);
}

/* Removes the last character of the dynamic string and updates null termination. */
void dstr_pop_back(DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}

	if (str->len == 0) {
		dstr_set_error_code(DSTR_ERROR_OUT_OF_BOUNDS);
		return;
	}

	str->len--;
	str->buffer[str->len] = '\0';
	dstr_set_error_code(DSTR_SUCCESS);
}

/* Searching */

/* Finds the index of the first occurrence of a substring needle within the string. Returns -1 if not found. */
int dstr_find(const DString *str, const DString *needle) {
	if (!str || !needle) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return -1;
	}

	if (needle->len == 0) return 0;
	if (needle->len > str->len) return -1;
	
	char *pos = strstr(str->buffer, needle->buffer);
	if (pos == NULL) {
		dstr_set_error_code(DSTR_ERROR_NOT_FOUND);
		return -1;
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return (int)(pos - str->buffer);
}

/* Finds the index of the first occurrence of a C-string needle within the string. Returns -1 if not found. */
int dstr_find_cstr(const DString *str, const char *needle) {
	if (!str || !needle) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return -1;
	}

	if (needle[0] == '\0') return 0;

	char *pos = strstr(str->buffer, needle);
	if (pos == NULL) {
		dstr_set_error_code(DSTR_ERROR_NOT_FOUND);
		return -1;
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return (int)(pos - str->buffer);
}

/* Finds the index of the last occurrence of a substring needle searching backward. Returns -1 if not found. */
int dstr_rfind(const DString *str, const DString *needle) {
	if (!str || !needle) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return -1;
	}

	if (needle->len == 0) return (int)str->len;
	if (needle->len > str->len) return -1;

	for (size_t i = str->len - needle->len + 1; i > 0; i--) {
		if (memcmp(str->buffer + i - 1, needle->buffer, needle->len) == 0) {
			dstr_set_error_code(DSTR_SUCCESS);
			return (int)(i - 1);
		}
	}

	dstr_set_error_code(DSTR_ERROR_NOT_FOUND);
	return -1;
}

/* Returns true if the needle substring exists anywhere inside the string. */
bool dstr_contains(const DString *str, const DString *needle) {
	return dstr_find(str, needle) != -1;
}

/* Checks if the string begins with the specified prefix substring. */
bool dstr_starts_with(const DString *str, const DString *prefix) {
	if (!str || !prefix) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return false;
	}

	if (prefix->len > str->len) return false;

	dstr_set_error_code(DSTR_SUCCESS);
	return memcmp(str->buffer, prefix->buffer, prefix->len) == 0;
}

/* Checks if the string ends with the specified suffix substring. */
bool dstr_ends_with(const DString *str, const DString *suffix) {
	if (!str || !suffix) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return false;
	}

	if (suffix->len > str->len) return false;

	dstr_set_error_code(DSTR_SUCCESS);
	return memcmp(str->buffer + (str->len - suffix->len), suffix->buffer, suffix->len) == 0;
}

/* Substrings */

/* Returns a new dynamic string extracted from a specific position and matching the given length. */
DString dstr_substr(const DString *str, size_t pos, size_t len) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return (DString){0};
	}

	if (pos >= str->len) {
		dstr_set_error_code(DSTR_ERROR_OUT_OF_BOUNDS);
		return dstr_create();
	}
	
	if (pos + len > str->len) len = str->len - pos;
	return dstr_from_buffer(str->buffer + pos, len);
}

/* Replacement */

/* Finds and replaces all occurrences of the 'old' substring with the 'replacement' substring. */
void dstr_replace(DString *str, const DString *old, const DString *replacement) {
	if (!str || !old || !replacement) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}

	if (old->len == 0) return;

	size_t search_from = 0;

	while (search_from <= str->len) {
		if (old->len > str->len - search_from) break;

		char *pos = strstr(str->buffer + search_from, old->buffer);
		if (pos == NULL) break;

		size_t idx = (size_t)(pos - str->buffer);

		dstr_erase(str, idx, old->len);
		if (g_dstr_last_error == DSTR_ERROR_ALLOC_FAILED) return;

		dstr_insert(str, idx, replacement);
		if (g_dstr_last_error == DSTR_ERROR_ALLOC_FAILED) return;

		search_from = idx + replacement->len;
	}

	dstr_set_error_code(DSTR_SUCCESS);
}

/* Utilities */

/* Removes all leading and trailing whitespace characters (spaces, tabs, newlines, carriage returns). */
void dstr_trim(DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}

	dstr_trim_left(str);
	dstr_trim_right(str);
}

/* Removes leading whitespace characters from the left side of the string. */
void dstr_trim_left(DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}

	size_t start = 0;
	while (start < str->len && (str->buffer[start] == ' ' || str->buffer[start] == '\t' || str->buffer[start] == '\n' || str->buffer[start] == '\r')) {
		start++;
	}

	if (start > 0) {
		memmove(str->buffer, str->buffer + start, str->len - start + 1);
		str->len -= start;
	}

	dstr_set_error_code(DSTR_SUCCESS);
}

/* Removes trailing whitespace characters from the right side of the string. */
void dstr_trim_right(DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}

	while (str->len > 0 && (str->buffer[str->len - 1] == ' ' || str->buffer[str->len - 1] == '\t' || str->buffer[str->len - 1] == '\n' || str->buffer[str->len - 1] == '\r')) {
		str->len--;
	}

	str->buffer[str->len] = '\0';
	dstr_set_error_code(DSTR_SUCCESS);
}

/* Reverses the order of the characters in the string in-place. */
void dstr_reverse(DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}

	if (str->len == 0) return;

	size_t i = 0;
	size_t j = str->len - 1;

	while (i < j) {
		char temp = str->buffer[i];
		str->buffer[i] = str->buffer[j];
		str->buffer[j] = temp;
		i++;
		j--;
	}

	dstr_set_error_code(DSTR_SUCCESS);
}

/* Converts all lowercase ASCII characters in the string to uppercase in-place. */
void dstr_to_upper(DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}

	for (size_t i = 0; i < str->len; i++) {
		if (str->buffer[i] >= 'a' && str->buffer[i] <= 'z') {
			str->buffer[i] -= 32;
		}
	}
	dstr_set_error_code(DSTR_SUCCESS);
}

/* Converts all uppercase ASCII characters in the string to lowercase in-place. */
void dstr_to_lower(DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}

	for (size_t i = 0; i < str->len; i++) {
		if (str->buffer[i] >= 'A' && str->buffer[i] <= 'Z') {
			str->buffer[i] += 32;
		}
	}

	dstr_set_error_code(DSTR_SUCCESS);
}

/* Counts and returns the total number of occurrences of a specific character in the string. */
size_t dstr_count(const DString *str, char ch) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return 0;
	}

	size_t count = 0;
	for (size_t i = 0; i < str->len; i++) {
		if (str->buffer[i] == ch)
			count++;
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return count;
}