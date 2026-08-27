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

void dstr_set_error_code(DStringError code) {
	if (code >= 0 && code < DSTR_ERROR_COUNT) {
		g_dstr_last_error = code;
	}
}

DStringError dstr_get_error_code(void) {
	return g_dstr_last_error;
}

const char *dstr_get_error_string(void) {
	return g_dstr_error_strings[g_dstr_last_error];
}

void dstr_clear_error_code(void) {
	g_dstr_last_error = DSTR_SUCCESS;
}

/* Construction / Destruction */

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

size_t dstr_len(const DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return 0;
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return str->len;
}

size_t dstr_capacity(const DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return 0;
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return str->capacity;
}

bool dstr_empty(const DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return true;
	}
	
	dstr_set_error_code(DSTR_SUCCESS);
	return str->len == 0;
}

void dstr_clear(DString *str) {
	if (!str || !str->buffer) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}

	str->len       = 0;
	str->buffer[0] = '\0';
	dstr_set_error_code(DSTR_SUCCESS);
}

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

char *dstr_data(DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return NULL;
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return str->buffer;
}

const char *dstr_cstr(const DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return NULL;
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return str->buffer;
}

/* Assignment */

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

bool dstr_equal(const DString *a, const DString *b) {
	if (!a || !b) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return false;
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return a->len == b->len && memcmp(a->buffer, b->buffer, a->len) == 0;
}

bool dstr_equal_cstr(const DString *a, const char *b) {
	if (!a || !b) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return false;
	}

	dstr_set_error_code(DSTR_SUCCESS);
	return a->len == strlen(b) && memcmp(a->buffer, b, a->len) == 0;
}

/* Append */

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

bool dstr_contains(const DString *str, const DString *needle) {
	return dstr_find(str, needle) != -1;
}

bool dstr_starts_with(const DString *str, const DString *prefix) {
	if (!str || !prefix) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return false;
	}

	if (prefix->len > str->len) return false;

	dstr_set_error_code(DSTR_SUCCESS);
	return memcmp(str->buffer, prefix->buffer, prefix->len) == 0;
}

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

void dstr_trim(DString *str) {
	if (!str) {
		dstr_set_error_code(DSTR_ERROR_NULL_ARGUMENT);
		return;
	}

	dstr_trim_left(str);
	dstr_trim_right(str);
}

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
