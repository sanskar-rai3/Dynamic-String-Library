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

#ifndef D_STRING_H
#define D_STRING_H

#include <stddef.h>
#include <stdbool.h>

typedef struct DString {
	char  *buffer;
	size_t len;
	size_t capacity;
} DString;

typedef enum DStringError {
	DSTR_SUCCESS = 0,
	DSTR_ERROR_ALLOC_FAILED,
	DSTR_ERROR_OUT_OF_BOUNDS,
	DSTR_ERROR_NULL_ARGUMENT,
	DSTR_ERROR_INVALID_SIZE,
	DSTR_ERROR_NOT_FOUND,
	DSTR_ERROR_COUNT
} DStringError;

/* Error Handling */

/* Sets the internal global error code if the provided code is valid.
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
void dstr_set_error_code(DStringError code);

/* Returns the most recently recorded global error code.
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
DStringError dstr_get_error_code(void);

/* Returns the descriptive text message corresponding to the current
 * global error code.
 *
 * Parameters: none.
 *
 * Returns: a pointer to a static, read-only string describing the
 *          current error code. Never NULL; DSTR_SUCCESS maps to "".
 */
const char *dstr_get_error_string(void);

/* Resets the global error status back to DSTR_SUCCESS.
 *
 * Parameters: none.
 *
 * Returns: nothing.
 */
void dstr_clear_error_code(void);

/* Construction / Destruction */

/* Initializes an empty dynamic string with a default capacity of 8 bytes.
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
DString dstr_create(void);

/* Creates a dynamic string by copying contents from a null-terminated
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
 *   DSTR_SUCCESS             - otherwise.
 */
DString dstr_from_cstr(const char *str);

/* Creates a dynamic string by copying a specified number of bytes from
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
 *   DSTR_SUCCESS             - otherwise.
 */
DString dstr_from_buffer(const char *buffer, size_t len);

/* Frees the allocated memory of the dynamic string and resets its
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
 *   DSTR_SUCCESS             - otherwise.
 */
void dstr_destroy(DString *str);

/* Capacity */

/* Returns the current length (number of characters) of the dynamic
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
 *   DSTR_SUCCESS             - otherwise.
 */
size_t dstr_len(const DString *str);

/* Returns the total allocated memory capacity of the dynamic string
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
 *   DSTR_SUCCESS             - otherwise.
 */
size_t dstr_capacity(const DString *str);

/* Checks if the string length is zero.
 *
 * Parameters:
 *   str - pointer to the DString to query. Must not be NULL.
 *
 * Returns: true if str's length is 0, or if str is NULL. false
 *          otherwise.
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str was NULL.
 *   DSTR_SUCCESS             - otherwise.
 */
bool dstr_empty(const DString *str);

/* Resets the string length to zero and null-terminates the start
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
 *   DSTR_SUCCESS             - otherwise.
 */
void dstr_clear(DString *str);

/* Expands the buffer capacity to at least the requested size if it is
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
 *   DSTR_SUCCESS             - otherwise.
 */
void dstr_reserve(DString *str, size_t capacity);

/* Resizes the string to a new length, padding with zeroes if it grows,
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
 *   DSTR_SUCCESS             - otherwise.
 */
void dstr_resize(DString *str, size_t new_len);

/* Shrinks the memory capacity down to exactly fit the current length
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
 *   DSTR_SUCCESS             - otherwise.
 */
void dstr_shrink_to_fit(DString *str);

/* Element Access */

/* Returns the character at the specified index with boundary checks.
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
 *   DSTR_SUCCESS                - otherwise.
 */
char dstr_at(const DString *str, size_t index);

/* Returns the first character of the string.
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
 *   DSTR_SUCCESS                - otherwise.
 */
char dstr_front(const DString *str);

/* Returns the last character of the string.
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
 *   DSTR_SUCCESS                - otherwise.
 */
char dstr_back(const DString *str);

/* Returns a mutable raw pointer to the underlying character buffer.
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
 *   DSTR_SUCCESS             - otherwise.
 */
char *dstr_data(DString *str);

/* Returns a read-only C-string pointer to the underlying buffer.
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
 *   DSTR_SUCCESS             - otherwise.
 */
const char *dstr_cstr(const DString *str);

/* Assignment */

/* Overwrites the contents of the destination string with a copy of the
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
 *   DSTR_SUCCESS             - otherwise.
 */
void dstr_copy(DString *dest, const DString *src);

/* Overwrites the contents of the destination string with a copy of a
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
 *   DSTR_SUCCESS             - otherwise.
 */
void dstr_copy_cstr(DString *dest, const char *src);

/* Swaps the internal buffer pointers, lengths, and capacities of two
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
 *   DSTR_SUCCESS             - otherwise.
 */
void dstr_swap(DString *a, DString *b);

/* Comparison */

/* Compares two dynamic strings lexicographically.
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
 *   DSTR_SUCCESS             - otherwise.
 */
int dstr_cmp(const DString *a, const DString *b);

/* Compares a dynamic string with a C-string lexicographically.
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
 *   DSTR_SUCCESS             - otherwise.
 */
int dstr_cmp_cstr(const DString *a, const char *b);

/* Checks if two dynamic strings are identical in length and content.
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
 *   DSTR_SUCCESS             - otherwise.
 */
bool dstr_equal(const DString *a, const DString *b);

/* Checks if a dynamic string is identical in content to a C-string.
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
 *   DSTR_SUCCESS             - otherwise.
 */
bool dstr_equal_cstr(const DString *a, const char *b);

/* Append */

/* Appends the contents of the source dynamic string to the end of the
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
 *   DSTR_SUCCESS             - otherwise.
 */
void dstr_append(DString *dest, const DString *src);

/* Appends a null-terminated C-string to the end of the destination
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
 *   DSTR_SUCCESS             - otherwise.
 */
void dstr_append_cstr(DString *dest, const char *src);

/* Appends a single character to the end of the string.
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
 *   DSTR_SUCCESS             - otherwise.
 */
void dstr_push_back(DString *str, char ch);

/* Insert / Erase */

/* Inserts a source dynamic string into the target string at the
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
 *   DSTR_SUCCESS             - otherwise.
 */
void dstr_insert(DString *str, size_t pos, const DString *src);

/* Inserts a C-string into the dynamic string at the specified
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
 *   DSTR_SUCCESS             - otherwise.
 */
void dstr_insert_cstr(DString *str, size_t pos, const char *src);

/* Inserts a single character into the dynamic string at the specified
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
 *   DSTR_SUCCESS             - otherwise.
 */
void dstr_insert_char(DString *str, size_t pos, char ch);

/* Erases a segment of characters from the string starting at a given
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
 *   DSTR_SUCCESS                - otherwise.
 */
void dstr_erase(DString *str, size_t pos, size_t len);

/* Removes the last character of the dynamic string and updates null
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
 *   DSTR_SUCCESS                - otherwise.
 */
void dstr_pop_back(DString *str);

/* Searching */

/* Finds the index of the first occurrence of a substring needle within
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
 *   DSTR_SUCCESS             - otherwise.
 */
int dstr_find(const DString *str, const DString *needle);

/* Finds the index of the first occurrence of a C-string needle within
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
 *   DSTR_SUCCESS             - otherwise.
 */
int dstr_find_cstr(const DString *str, const char *needle);

/* Finds the index of the last occurrence of a substring needle
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
 *   DSTR_SUCCESS             - otherwise.
 */
int dstr_rfind(const DString *str, const DString *needle);

/* Returns true if the needle substring exists anywhere inside the
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
bool dstr_contains(const DString *str, const DString *needle);

/* Checks if the string begins with the specified prefix substring.
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
 *   DSTR_SUCCESS             - otherwise.
 */
bool dstr_starts_with(const DString *str, const DString *prefix);

/* Checks if the string ends with the specified suffix substring.
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
 *   DSTR_SUCCESS             - otherwise.
 */
bool dstr_ends_with(const DString *str, const DString *suffix);

/* Substrings */

/* Returns a new dynamic string extracted from a specific position and
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
 *   DSTR_SUCCESS                - otherwise.
 */
DString dstr_substr(const DString *str, size_t pos, size_t len);

/* Replacement */

/* Finds and replaces all occurrences of the 'old' substring with the
 * 'replacement' substring.
 *
 * Parameters:
 *   str         - pointer to the DString to modify. Must not be NULL.
 *   old         - pointer to the DString to search for. Must not be
 *                 NULL. If empty, this call is a no-op.
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
 *   DSTR_SUCCESS             - otherwise.
 */
void dstr_replace(DString *str,
                  const DString *old,
                  const DString *replacement);

/* Utilities */

/* Removes all leading and trailing whitespace characters (spaces,
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
 *   DSTR_SUCCESS             - otherwise.
 */
void dstr_trim(DString *str);

/* Removes leading whitespace characters from the left side of the
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
 *   DSTR_SUCCESS             - otherwise.
 */
void dstr_trim_left(DString *str);

/* Removes trailing whitespace characters from the right side of the
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
 *   DSTR_SUCCESS             - otherwise.
 */
void dstr_trim_right(DString *str);

/* Reverses the order of the characters in the string in-place.
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
 *   DSTR_SUCCESS             - otherwise.
 */
void dstr_reverse(DString *str);

/* Converts all lowercase ASCII characters in the string to uppercase
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
 *   DSTR_SUCCESS             - otherwise.
 */
void dstr_to_upper(DString *str);

/* Converts all uppercase ASCII characters in the string to lowercase
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
 *   DSTR_SUCCESS             - otherwise.
 */
void dstr_to_lower(DString *str);

/* Counts and returns the total number of occurrences of a specific
 * character in the string.
 *
 * Parameters:
 *   str - pointer to the DString to search. Must not be NULL.
 *   ch  - the character to count.
 *
 * Returns: the number of times ch appears in str. Returns 0 if str is
 *        NULL (indistinguishable from a legitimate count of 0;
 *        check the error code if that distinction matters).
 *
 * Error codes set:
 *   DSTR_ERROR_NULL_ARGUMENT - str was NULL.
 *   DSTR_SUCCESS              - otherwise.
 */
size_t dstr_count(const DString *str, char ch);

#endif /* D_STRING_H */
