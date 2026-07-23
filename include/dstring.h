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

#ifndef __D_STRING__
#define __D_STRING__

#include <stddef.h>
#include <stdbool.h>

typedef struct DString {
	char   *buffer;
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
void dstr_set_error_code(DStringError code);
DStringError dstr_get_error_code(void);
const char *dstr_get_error_string(void);
void dstr_clear_error_code(void);

/* Construction / Destruction */
DString dstr_create(void);
DString dstr_from_cstr(const char *str);
DString dstr_from_buffer(const char *buffer, size_t len);
void    dstr_destroy(DString *str);

/* Capacity */
size_t dstr_len(const DString *str);
size_t dstr_capacity(const DString *str);
bool   dstr_empty(const DString *str);
void   dstr_clear(DString *str);
void   dstr_reserve(DString *str, size_t capacity);
void   dstr_resize(DString *str, size_t new_len);
void   dstr_shrink_to_fit(DString *str);

/* Element Access */
char        dstr_at(const DString *str, size_t index);
char        dstr_front(const DString *str);
char        dstr_back(const DString *str);
char       *dstr_data(DString *str);
const char *dstr_cstr(const DString *str);

/* Assignment */
void dstr_copy(DString *dest, const DString *src);
void dstr_copy_cstr(DString *dest, const char *src);
void dstr_swap(DString *a, DString *b);

/* Comparison */
int  dstr_cmp(const DString *a, const DString *b);
int  dstr_cmp_cstr(const DString *a, const char *b);
bool dstr_equal(const DString *a, const DString *b);
bool dstr_equal_cstr(const DString *a, const char *b);

/* Append */
void dstr_append(DString *dest, const DString *src);
void dstr_append_cstr(DString *dest, const char *src);
void dstr_push_back(DString *str, char ch);

/* Insert / Erase */
void dstr_insert(DString *str, size_t pos, const DString *src);
void dstr_insert_cstr(DString *str, size_t pos, const char *src);
void dstr_insert_char(DString *str, size_t pos, char ch);
void dstr_erase(DString *str, size_t pos, size_t len);
void dstr_pop_back(DString *str);

/* Searching */
int  dstr_find(const DString *str, const DString *needle);
int  dstr_find_cstr(const DString *str, const char *needle);
int  dstr_rfind(const DString *str, const DString *needle);
bool dstr_contains(const DString *str, const DString *needle);
bool dstr_starts_with(const DString *str, const DString *prefix);
bool dstr_ends_with(const DString *str, const DString *suffix);

/* Substrings */
DString dstr_substr(const DString *str, size_t pos, size_t len);

/* Replacement */
void dstr_replace(DString *str,
                  const DString *old,
                  const DString *replacement);

/* Utilities */
void   dstr_trim(DString *str);
void   dstr_trim_left(DString *str);
void   dstr_trim_right(DString *str);
void   dstr_reverse(DString *str);
void   dstr_to_upper(DString *str);
void   dstr_to_lower(DString *str);
size_t dstr_count(const DString *str, char ch);

#endif
