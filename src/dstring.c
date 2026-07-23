/*
 *MIT License
 *
 *Copyright (c) 2026 Sanskar Rai
 *
 *Permission is hereby granted, free of charge, to any person obtaining a copy
 *of this software and associated documentation files (the "Software"), to deal
 *in the Software without restriction, including without limitation the rights
 *to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *copies of the Software, and to permit persons to whom the Software is
 *furnished to do so, subject to the following conditions:
 *
 *The above copyright notice and this permission notice shall be included in all
 *copies or substantial portions of the Software.
 *
 *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *SOFTWARE.
 */

#include "dstring.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef Thread_Local
    #if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
        #define Thread_Local _Thread_local
    #elif defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER)
        #define Thread_Local __thread
    #elif defined(_MSC_VER)
        #define Thread_Local __declspec(thread)
    #else
        #define Thread_Local static
        #warning "Compiler thread-local storage support undetected. Error handling fallback applied."
    #endif
#endif

/* Static Datas */
static const char *const g_dstr_error_strings[] = {
	[DSTR_SUCCESS]             = "",
	[DSTR_ERROR_ALLOC_FAILED]  = "Memory allocation or reallocation failed",
	[DSTR_ERROR_OUT_OF_BOUNDS] = "Index or position out of bounds",
	[DSTR_ERROR_NULL_ARGUMENT] = "Null pointer passed as a required argument",
	[DSTR_ERROR_INVALID_SIZE]  = "Requested size or capacity change is invalid",
	[DSTR_ERROR_NOT_FOUND]     = "Substring or character target not found"
};


static Thread_Local DStringError g_dstr_last_error = DSTR_SUCCESS;

/* Error Handling */

/*
 * Sets the internal global error code if the provided code is valid.
 *
 * Parameters:
 *   code - the error code to record. Must be one of the DStringError
 *          enum values (0 to DSTR_ERROR_COUNT - 1).
 *
 * Returns: nothing.
 *
 * Notes:
 *   If code is out of range, the current error code is left unchanged.
 *   This is the single write path used by every other function in this
 *   file to report its outcome, so calling it directly is rarely needed
 *   outside of custom extensions to the library.
 */
void dstr_set_error_code(DStringError code) {
	if (code >= 0 && code < DSTR_ERROR_COUNT) {
		g_dstr_last_error = code;
	}
}

/*
 * Returns the most recently recorded global error code.
 *
 * Parameters: none.
 *
 * Returns: the DStringError set by the last dstr_* call.
 *
 * Notes:
 *   Almost every function in this library, including read-only ones
 *   like dstr_len(), overwrites this value on its own success path.
 *   Check it immediately after the call you care about, before making
 *   any other dstr_* call, or the result you want can be overwritten.
 */
DStringError dstr_get_error_code(void) {
	return g_dstr_last_error;
}

/*
 * Returns the descriptive text message corresponding to the current
 * global error code.
 *
 * Parameters: none.
 *
 * Returns: a pointer to a static, read-only string describing the
 *          current error code. Never NULL; DSTR_SUCCESS maps to "".
 */
const char *dstr_get_error_string(void) {
	return g_dstr_error_strings[g_dstr_last_error];
}

/*
 * Resets the global error status back to DSTR_SUCCESS.
 *
 * Parameters: none.
 *
 * Returns: nothing.
 */
void dstr_clear_error_code(void) {
	g_dstr_last_error = DSTR_SUCCESS;
}

/* Construction / Destruction */

/*
 * Initializes an empty dynamic string with a default capacity of 8 bytes.
 *
 * Parameters: none.
 *
 * Returns: a new DString with len 0. On allocation failure, returns a
 *          zeroed DString ({0}) whose buffer is NULL.
 *
 * Error codes set:
 *   DSTR_ERROR_ALLOC_FAILED - the initial malloc failed.
 *   DSTR_SUCCESS             - otherwise.
 */
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

/*
 * Creates a dynamic string by copying contents from a null-terminated
 * C-string.
 *
 * Parameters:
 *   str - a null-terminated C-string to copy from. Must not be NULL.
 *
 * Returns: a new DString containing a copy of str. On failure (NULL
 *          argument or allocation failure), returns a zeroed DString
 *          ({0}) whose buffer is NULL.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str was NULL.
 *   DSTR_ERROR_ALLOC_FAILED  - the buffer allocation failed.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Creates a dynamic string by copying a specified number of bytes from
 * a raw buffer.
 *
 * Parameters:
 *   buffer - pointer to the raw bytes to copy from. Must not be NULL.
 *            Does not need to be null-terminated; embedded '\0' bytes
 *            within the first len bytes are copied as ordinary data.
 *   len    - number of bytes to copy from buffer.
 *
 * Returns: a new DString containing a copy of the first len bytes of
 *          buffer, always null-terminated after those bytes. On failure
 *          (NULL argument or allocation failure), returns a zeroed
 *          DString ({0}) whose buffer is NULL.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - buffer was NULL.
 *   DSTR_ERROR_ALLOC_FAILED  - the buffer allocation failed.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Frees the allocated memory of the dynamic string and resets its
 * properties to zero.
 *
 * Parameters:
 *   str - pointer to the DString to destroy. Must not be NULL.
 *
 * Returns: nothing.
 *
 * Notes:
 *   Safe to call on a DString that was already destroyed (its buffer
 *   will be NULL, and free(NULL) is a no-op). After this call, str->len
 *   and str->capacity are both 0 and str->buffer is NULL.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str was NULL.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Returns the current length (number of characters) of the dynamic
 * string.
 *
 * Parameters:
 *   str - pointer to the DString to query. Must not be NULL.
 *
 * Returns: the number of characters currently stored in str, not
 *          counting the null terminator. Returns 0 if str is NULL.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str was NULL.
 *   DSTR_SUCCESS              - otherwise.
 */
size_t dstr_len(const DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return 0;
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return str->len;
}

/*
 * Returns the total allocated memory capacity of the dynamic string
 * buffer.
 *
 * Parameters:
 *   str - pointer to the DString to query. Must not be NULL.
 *
 * Returns: the total number of bytes currently allocated for str's
 *          buffer, including room for the null terminator. Returns 0
 *          if str is NULL.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str was NULL.
 *   DSTR_SUCCESS              - otherwise.
 */
size_t dstr_capacity(const DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return 0;
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return str->capacity;
}

/*
 * Checks if the string length is zero.
 *
 * Parameters:
 *   str - pointer to the DString to query. Must not be NULL.
 *
 * Returns: true if str's length is 0, or if str is NULL. false
 *          otherwise.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str was NULL.
 *   DSTR_SUCCESS              - otherwise.
 */
bool dstr_empty(const DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return true;
	}
	
	dstr_set_error_code(DSTR_SUCCESS);
	return str->len == 0;
}

/*
 * Resets the string length to zero and null-terminates the start
 * without freeing memory.
 *
 * Parameters:
 *   str - pointer to the DString to clear. Must not be NULL, and its
 *         buffer must not be NULL (i.e. it must be a live, constructed
 *         DString, not a destroyed or zero-initialized one).
 *
 * Returns: nothing.
 *
 * Notes:
 *   The allocated capacity is left unchanged, so the buffer can be
 *   reused without another allocation on the next append/insert.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str was NULL, or str->buffer was NULL.
 *   DSTR_SUCCESS              - otherwise.
 */
void dstr_clear(DString *str) {
	if (!str || !str->buffer) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}

	str->len       = 0;
	str->buffer[0] = '\0';
	dstr_set_error_code(DSTR_SUCCESS);
}

/*
 * Expands the buffer capacity to at least the requested size if it is
 * greater than current capacity.
 *
 * Parameters:
 *   str      - pointer to the DString to reserve capacity for. Must
 *              not be NULL.
 *   capacity - the minimum total buffer size, in bytes, to guarantee.
 *
 * Returns: nothing.
 *
 * Notes:
 *   If capacity is less than or equal to the string's current
 *   capacity, this is a no-op and always succeeds. Growing does not
 *   change str->len or the string's contents; it only guarantees room
 *   for future writes.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str was NULL.
 *   DSTR_ERROR_ALLOC_FAILED  - the realloc failed; str is left
 *                              unmodified and still valid.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Resizes the string to a new length, padding with zeroes if it grows,
 * and updates null termination.
 *
 * Parameters:
 *   str     - pointer to the DString to resize. Must not be NULL.
 *   new_len - the desired new length, in characters.
 *
 * Returns: nothing.
 *
 * Notes:
 *   If new_len is greater than the current length, the newly exposed
 *   bytes are zero-filled (not left as garbage). If new_len is smaller,
 *   the string is truncated in place and the buffer is re-terminated;
 *   no reallocation happens when shrinking.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str was NULL.
 *   DSTR_ERROR_ALLOC_FAILED  - growing required a realloc that failed;
 *                              str is left unmodified and still valid.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Shrinks the memory capacity down to exactly fit the current length
 * plus the null terminator.
 *
 * Parameters:
 *   str - pointer to the DString to shrink. Must not be NULL.
 *
 * Returns: nothing.
 *
 * Notes:
 *   Does not change str->len or the string's contents, only its
 *   allocated capacity. Useful after a large number of erase/pop_back
 *   calls to release unused memory.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str was NULL.
 *   DSTR_ERROR_ALLOC_FAILED  - the realloc failed; str is left
 *                              unmodified and still valid.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Returns the character at the specified index with boundary checks.
 *
 * Parameters:
 *   str   - pointer to the DString to query. Must not be NULL.
 *   index - zero-based character index. Must be less than str's
 *           length.
 *
 * Returns: the character at index. Returns '\0' if str is NULL or
 *          index is out of bounds - which is indistinguishable from a
 *          legitimately stored '\0' character. Check the error code if
 *          that distinction matters.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT  - str was NULL.
 *   DSTR_ERROR_OUT_OF_BOUNDS  - index >= str->len.
 *   DSTR_SUCCESS               - otherwise.
 */
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

/*
 * Returns the first character of the string.
 *
 * Parameters:
 *   str - pointer to the DString to query. Must not be NULL.
 *
 * Returns: the first character of str. Returns '\0' if str is NULL or
 *          empty - which is indistinguishable from a legitimately
 *          stored '\0' character. Check the error code if that
 *          distinction matters.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT  - str was NULL.
 *   DSTR_ERROR_OUT_OF_BOUNDS  - str's length is 0.
 *   DSTR_SUCCESS               - otherwise.
 */
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

/*
 * Returns the last character of the string.
 *
 * Parameters:
 *   str - pointer to the DString to query. Must not be NULL.
 *
 * Returns: the last character of str. Returns '\0' if str is NULL or
 *          empty - which is indistinguishable from a legitimately
 *          stored '\0' character. Check the error code if that
 *          distinction matters.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT  - str was NULL.
 *   DSTR_ERROR_OUT_OF_BOUNDS  - str's length is 0.
 *   DSTR_SUCCESS               - otherwise.
 */
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

/*
 * Returns a mutable raw pointer to the underlying character buffer.
 *
 * Parameters:
 *   str - pointer to the DString to access. Must not be NULL.
 *
 * Returns: a pointer to str's internal buffer, or NULL if str is NULL.
 *
 * Notes:
 *   The returned pointer is only valid until the next call that may
 *   reallocate str's buffer (append, insert, resize, reserve,
 *   shrink_to_fit, copy, etc.) or until str is destroyed. Writing past
 *   str->len bytes, or past str->capacity - 1 bytes, is undefined
 *   behavior; use dstr_resize() first if more room is needed. Direct
 *   writes through this pointer do not update str->len automatically.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str was NULL.
 *   DSTR_SUCCESS              - otherwise.
 */
char *dstr_data(DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return NULL;
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return str->buffer;
}

/*
 * Returns a read-only C-string pointer to the underlying buffer.
 *
 * Parameters:
 *   str - pointer to the DString to access. Must not be NULL.
 *
 * Returns: a null-terminated, read-only pointer to str's internal
 *          buffer, or NULL if str is NULL.
 *
 * Notes:
 *   The returned pointer is only valid until the next call that may
 *   reallocate str's buffer, or until str is destroyed - do not store
 *   it past that point.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str was NULL.
 *   DSTR_SUCCESS              - otherwise.
 */
const char *dstr_cstr(const DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return NULL;
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return str->buffer;
}

/* Assignment */

/*
 * Overwrites the contents of the destination string with a copy of the
 * source string.
 *
 * Parameters:
 *   dest - pointer to the DString to overwrite. Must not be NULL.
 *   src  - pointer to the DString to copy from. Must not be NULL. May
 *          alias dest (copying a string onto itself is a safe no-op
 *          in effect).
 *
 * Returns: nothing.
 *
 * Notes:
 *   This is a deep copy: dest ends up with its own buffer containing
 *   the same bytes as src, and src is left unmodified. dest's existing
 *   buffer is grown via realloc if it isn't already large enough, and
 *   reused (not freed and reallocated from scratch) if it is.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - dest or src was NULL.
 *   DSTR_ERROR_ALLOC_FAILED  - growing dest's buffer failed; dest is
 *                              left unmodified and still valid.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Overwrites the contents of the destination string with a copy of a
 * null-terminated C-string.
 *
 * Parameters:
 *   dest - pointer to the DString to overwrite. Must not be NULL.
 *   src  - a null-terminated C-string to copy from. Must not be NULL.
 *
 * Returns: nothing.
 *
 * Notes:
 *   dest's existing buffer is grown via realloc if it isn't already
 *   large enough, and reused if it is.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - dest or src was NULL.
 *   DSTR_ERROR_ALLOC_FAILED  - growing dest's buffer failed; dest is
 *                              left unmodified and still valid.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Swaps the internal buffer pointers, lengths, and capacities of two
 * strings.
 *
 * Parameters:
 *   a - pointer to the first DString. Must not be NULL.
 *   b - pointer to the second DString. Must not be NULL.
 *
 * Returns: nothing.
 *
 * Notes:
 *   This is a cheap, allocation-free swap: no bytes are copied, only
 *   the struct fields are exchanged. After this call, a holds what b
 *   held and vice versa.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - a or b was NULL.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Compares two dynamic strings lexicographically.
 *
 * Parameters:
 *   a - pointer to the first DString. Must not be NULL.
 *   b - pointer to the second DString. Must not be NULL.
 *
 * Returns: a negative value if a < b, 0 if a and b are equal, or a
 *          positive value if a > b, byte-wise as unsigned char.
 *          Returns 0 if either argument is NULL.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - a or b was NULL.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Compares a dynamic string with a C-string lexicographically.
 *
 * Parameters:
 *   a - pointer to the DString. Must not be NULL.
 *   b - a null-terminated C-string to compare against. Must not be
 *       NULL.
 *
 * Returns: a negative value if a < b, 0 if they are equal, or a
 *          positive value if a > b, byte-wise as unsigned char.
 *          Returns 0 if either argument is NULL.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - a or b was NULL.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Checks if two dynamic strings are identical in length and content.
 *
 * Parameters:
 *   a - pointer to the first DString. Must not be NULL.
 *   b - pointer to the second DString. Must not be NULL.
 *
 * Returns: true if a and b have the same length and identical bytes.
 *          Returns false if either argument is NULL.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - a or b was NULL.
 *   DSTR_SUCCESS              - otherwise.
 */
bool dstr_equal(const DString *a, const DString *b) {
	if (!a || !b) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return false;
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return a->len == b->len && memcmp(a->buffer, b->buffer, a->len) == 0;
}

/*
 * Checks if a dynamic string is identical in content to a C-string.
 *
 * Parameters:
 *   a - pointer to the DString. Must not be NULL.
 *   b - a null-terminated C-string to compare against. Must not be
 *       NULL.
 *
 * Returns: true if a's content exactly matches b. Returns false if
 *          either argument is NULL.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - a or b was NULL.
 *   DSTR_SUCCESS              - otherwise.
 */
bool dstr_equal_cstr(const DString *a, const char *b) {
	if (!a || !b) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return false;
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return a->len == strlen(b) && memcmp(a->buffer, b, a->len) == 0;
}

/* Append */

/*
 * Appends the contents of the source dynamic string to the end of the
 * destination string.
 *
 * Parameters:
 *   dest - pointer to the DString to append to. Must not be NULL.
 *   src  - pointer to the DString to append. Must not be NULL.
 *
 * Returns: nothing.
 *
 * Notes:
 *   dest's buffer is grown (geometric doubling) if there isn't enough
 *   room. src is left unmodified.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - dest or src was NULL.
 *   DSTR_ERROR_ALLOC_FAILED  - growing dest's buffer failed; dest is
 *                              left unmodified and still valid.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Appends a null-terminated C-string to the end of the destination
 * dynamic string.
 *
 * Parameters:
 *   dest - pointer to the DString to append to. Must not be NULL.
 *   src  - a null-terminated C-string to append. Must not be NULL.
 *
 * Returns: nothing.
 *
 * Notes:
 *   dest's buffer is grown (geometric doubling) if there isn't enough
 *   room.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - dest or src was NULL.
 *   DSTR_ERROR_ALLOC_FAILED  - growing dest's buffer failed; dest is
 *                              left unmodified and still valid.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Appends a single character to the end of the string.
 *
 * Parameters:
 *   str - pointer to the DString to append to. Must not be NULL.
 *   ch  - the character to append.
 *
 * Returns: nothing.
 *
 * Notes:
 *   str's buffer is grown (geometric doubling) if there isn't enough
 *   room.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str was NULL.
 *   DSTR_ERROR_ALLOC_FAILED  - growing str's buffer failed; str is
 *                              left unmodified and still valid.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Inserts a source dynamic string into the target string at the
 * specified position.
 *
 * Parameters:
 *   str - pointer to the DString to insert into. Must not be NULL.
 *   pos - zero-based insertion position. Clamped to str->len if it is
 *         greater than str's current length (so inserting past the
 *         end just appends).
 *   src - pointer to the DString to insert. Must not be NULL.
 *
 * Returns: nothing.
 *
 * Notes:
 *   str's buffer is grown if there isn't enough room. Existing
 *   characters at and after pos are shifted right to make room. src is
 *   left unmodified.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str or src was NULL.
 *   DSTR_ERROR_ALLOC_FAILED  - growing str's buffer failed; str is
 *                              left unmodified and still valid.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Inserts a C-string into the dynamic string at the specified
 * position.
 *
 * Parameters:
 *   str - pointer to the DString to insert into. Must not be NULL.
 *   pos - zero-based insertion position. Clamped to str->len if it is
 *         greater than str's current length (so inserting past the
 *         end just appends).
 *   src - a null-terminated C-string to insert. Must not be NULL.
 *
 * Returns: nothing.
 *
 * Notes:
 *   str's buffer is grown if there isn't enough room. Existing
 *   characters at and after pos are shifted right to make room.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str or src was NULL.
 *   DSTR_ERROR_ALLOC_FAILED  - growing str's buffer failed; str is
 *                              left unmodified and still valid.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Inserts a single character into the dynamic string at the specified
 * position.
 *
 * Parameters:
 *   str - pointer to the DString to insert into. Must not be NULL.
 *   pos - zero-based insertion position. Clamped to str->len if it is
 *         greater than str's current length (so inserting past the
 *         end just appends).
 *   ch  - the character to insert.
 *
 * Returns: nothing.
 *
 * Notes:
 *   str's buffer is grown (geometric doubling) if there isn't enough
 *   room. Existing characters at and after pos are shifted right to
 *   make room.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str was NULL.
 *   DSTR_ERROR_ALLOC_FAILED  - growing str's buffer failed; str is
 *                              left unmodified and still valid.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Erases a segment of characters from the string starting at a given
 * position and length.
 *
 * Parameters:
 *   str - pointer to the DString to erase from. Must not be NULL.
 *   pos - zero-based starting position. Must be less than str's
 *         current length.
 *   len - number of characters to remove. If pos + len goes past the
 *         end of the string, it is clamped so only the remaining
 *         characters from pos to the end are removed.
 *
 * Returns: nothing.
 *
 * Notes:
 *   Never reallocates; the buffer's capacity is unchanged, only str->len
 *   shrinks and the remaining characters are shifted left to close the
 *   gap.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT  - str was NULL.
 *   DSTR_ERROR_OUT_OF_BOUNDS  - pos >= str->len.
 *   DSTR_SUCCESS               - otherwise.
 */
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

/*
 * Removes the last character of the dynamic string and updates null
 * termination.
 *
 * Parameters:
 *   str - pointer to the DString to modify. Must not be NULL.
 *
 * Returns: nothing.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT  - str was NULL.
 *   DSTR_ERROR_OUT_OF_BOUNDS  - str's length was already 0.
 *   DSTR_SUCCESS               - otherwise.
 */
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

/*
 * Finds the index of the first occurrence of a substring needle within
 * the string.
 *
 * Parameters:
 *   str    - pointer to the DString to search in. Must not be NULL.
 *   needle - pointer to the DString to search for. Must not be NULL.
 *            An empty needle matches at index 0.
 *
 * Returns: the zero-based index of the first occurrence of needle in
 *          str, or -1 if not found (including when either argument is
 *          NULL).
 *
 * Notes:
 *   Implemented with strstr internally, so if needle contains an
 *   embedded '\0' byte, only the portion before that byte is actually
 *   searched for.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str or needle was NULL.
 *   DSTR_ERROR_NOT_FOUND     - needle does not occur in str.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Finds the index of the first occurrence of a C-string needle within
 * the string.
 *
 * Parameters:
 *   str    - pointer to the DString to search in. Must not be NULL.
 *   needle - a null-terminated C-string to search for. Must not be
 *            NULL. An empty string matches at index 0.
 *
 * Returns: the zero-based index of the first occurrence of needle in
 *          str, or -1 if not found (including when either argument is
 *          NULL).
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str or needle was NULL.
 *   DSTR_ERROR_NOT_FOUND     - needle does not occur in str.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Finds the index of the last occurrence of a substring needle
 * searching backward.
 *
 * Parameters:
 *   str    - pointer to the DString to search in. Must not be NULL.
 *   needle - pointer to the DString to search for. Must not be NULL.
 *            An empty needle matches at index str->len.
 *
 * Returns: the zero-based index of the last occurrence of needle in
 *          str, or -1 if not found (including when either argument is
 *          NULL).
 *
 * Notes:
 *   Implemented with memcmp over needle's full tracked length, so
 *   unlike dstr_find, an embedded '\0' byte in needle does not cause
 *   early truncation of the search.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str or needle was NULL.
 *   DSTR_ERROR_NOT_FOUND     - needle does not occur in str.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Returns true if the needle substring exists anywhere inside the
 * string.
 *
 * Parameters:
 *   str    - pointer to the DString to search in. Must not be NULL.
 *   needle - pointer to the DString to search for. Must not be NULL.
 *
 * Returns: true if needle occurs anywhere in str, false otherwise
 *          (including when either argument is NULL).
 *
 * Notes:
 *   Implemented in terms of dstr_find, so the same error code is set
 *   as a side effect (DSTR_ERROR_NOT_FOUND is not itself a failure of
 *   this function; it just reflects that the substring was absent).
 */
bool dstr_contains(const DString *str, const DString *needle) {
	return dstr_find(str, needle) != -1;
}

/*
 * Checks if the string begins with the specified prefix substring.
 *
 * Parameters:
 *   str    - pointer to the DString to check. Must not be NULL.
 *   prefix - pointer to the DString to check for. Must not be NULL.
 *
 * Returns: true if str starts with prefix, false otherwise (including
 *          when either argument is NULL, or when prefix is longer than
 *          str).
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str or prefix was NULL.
 *   DSTR_SUCCESS              - otherwise.
 */
bool dstr_starts_with(const DString *str, const DString *prefix) {
	if (!str || !prefix) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return false;
	}

	if (prefix->len > str->len) return false;

	dstr_set_error_code(DSTR_SUCCESS);
	return memcmp(str->buffer, prefix->buffer, prefix->len) == 0;
}

/*
 * Checks if the string ends with the specified suffix substring.
 *
 * Parameters:
 *   str    - pointer to the DString to check. Must not be NULL.
 *   suffix - pointer to the DString to check for. Must not be NULL.
 *
 * Returns: true if str ends with suffix, false otherwise (including
 *          when either argument is NULL, or when suffix is longer than
 *          str).
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str or suffix was NULL.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Returns a new dynamic string extracted from a specific position and
 * matching the given length.
 *
 * Parameters:
 *   str - pointer to the DString to extract from. Must not be NULL.
 *   pos - zero-based starting position. Must be less than str's
 *         current length.
 *   len - number of characters to extract. If pos + len goes past the
 *         end of the string, it is clamped so only the remaining
 *         characters from pos to the end are extracted.
 *
 * Returns: a new DString containing a copy of the requested slice. If
 *          str is NULL, returns a zeroed DString ({0}). If pos is out
 *          of bounds, returns a new empty DString (as if from
 *          dstr_create()), not a zeroed one.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT  - str was NULL.
 *   DSTR_ERROR_OUT_OF_BOUNDS  - pos >= str->len.
 *   DSTR_ERROR_ALLOC_FAILED   - the new string's buffer allocation
 *                               failed.
 *   DSTR_SUCCESS               - otherwise.
 */
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

/*
 * Finds and replaces all occurrences of the 'old' substring with the
 * 'replacement' substring.
 *
 * Parameters:
 *   str         - pointer to the DString to modify. Must not be NULL.
 *   old          - pointer to the DString to search for. Must not be
 *                  NULL. If empty, this call is a no-op.
 *   replacement - pointer to the DString to substitute in place of
 *                 each match. Must not be NULL. May be empty, which
 *                 deletes each occurrence of old.
 *
 * Returns: nothing.
 *
 * Notes:
 *   Matches are found left to right and are non-overlapping: after a
 *   replacement, the search resumes immediately after the inserted
 *   replacement text, so replacement is never itself re-scanned for
 *   further matches of old. Implemented with strstr internally (via
 *   dstr_find's underlying mechanism), so an embedded '\0' byte in old
 *   causes matching to stop at that byte.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str, old, or replacement was NULL.
 *   DSTR_ERROR_ALLOC_FAILED  - a buffer growth during replacement
 *                              failed; str may be left partially
 *                              modified (some earlier matches already
 *                              replaced) if this occurs partway
 *                              through.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Removes all leading and trailing whitespace characters (spaces,
 * tabs, newlines, carriage returns).
 *
 * Parameters:
 *   str - pointer to the DString to trim. Must not be NULL.
 *
 * Returns: nothing.
 *
 * Notes:
 *   Equivalent to calling dstr_trim_left() followed by
 *   dstr_trim_right(). ASCII whitespace only (' ', '\t', '\n', '\r');
 *   no locale or Unicode awareness.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str was NULL.
 *   DSTR_SUCCESS              - otherwise.
 */
void dstr_trim(DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}

	dstr_trim_left(str);
	dstr_trim_right(str);
}

/*
 * Removes leading whitespace characters from the left side of the
 * string.
 *
 * Parameters:
 *   str - pointer to the DString to trim. Must not be NULL.
 *
 * Returns: nothing.
 *
 * Notes:
 *   ASCII whitespace only (' ', '\t', '\n', '\r'). Never reallocates;
 *   remaining characters are shifted left in place.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str was NULL.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Removes trailing whitespace characters from the right side of the
 * string.
 *
 * Parameters:
 *   str - pointer to the DString to trim. Must not be NULL.
 *
 * Returns: nothing.
 *
 * Notes:
 *   ASCII whitespace only (' ', '\t', '\n', '\r'). Never reallocates;
 *   only str->len shrinks and the buffer is re-terminated.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str was NULL.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Reverses the order of the characters in the string in-place.
 *
 * Parameters:
 *   str - pointer to the DString to reverse. Must not be NULL.
 *
 * Returns: nothing.
 *
 * Notes:
 *   Operates byte-wise, so multi-byte encodings such as UTF-8 will
 *   have their individual bytes reordered, not their characters -
 *   reversing a string containing multi-byte UTF-8 sequences will
 *   corrupt them.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str was NULL.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Converts all lowercase ASCII characters in the string to uppercase
 * in-place.
 *
 * Parameters:
 *   str - pointer to the DString to convert. Must not be NULL.
 *
 * Returns: nothing.
 *
 * Notes:
 *   ASCII letters only ('a'-'z'); no locale or Unicode awareness.
 *   Non-ASCII bytes, including multi-byte UTF-8 sequences, pass
 *   through unmodified.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str was NULL.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Converts all uppercase ASCII characters in the string to lowercase
 * in-place.
 *
 * Parameters:
 *   str - pointer to the DString to convert. Must not be NULL.
 *
 * Returns: nothing.
 *
 * Notes:
 *   ASCII letters only ('A'-'Z'); no locale or Unicode awareness.
 *   Non-ASCII bytes, including multi-byte UTF-8 sequences, pass
 *   through unmodified.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str was NULL.
 *   DSTR_SUCCESS              - otherwise.
 */
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

/*
 * Counts and returns the total number of occurrences of a specific
 * character in the string.
 *
 * Parameters:
 *   str - pointer to the DString to search. Must not be NULL.
 *   ch  - the character to count.
 *
 * Returns: the number of times ch appears in str. Returns 0 if str is
 *          NULL (indistinguishable from a legitimate count of 0;
 *          check the error code if that distinction matters).
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str was NULL.
 *   DSTR_SUCCESS              - otherwise.
 */
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