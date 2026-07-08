#include "dstring.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* Construction / Destruction */
DString dstr_create(void) {
	size_t len      = 0;
	size_t capacity = 8;

	char *buffer = malloc(capacity);
	buffer[len] = '\0';

	return (DString) {
		.buffer   = buffer,
		.len      = len,
		.capacity = capacity
	};
}

DString dstr_from_cstr(const char *str) {
	size_t len   = strlen(str);

	size_t capacity = 8;
	while (capacity < len + 1) {
		capacity *= 2;
	}

	char *buffer = malloc(capacity); 
	memcpy(buffer, str, len + 1);

	return (DString){
		.buffer   = buffer,
		.len      = len,
		.capacity = capacity
	};
}
DString dstr_from_buffer(const char *buffer, size_t len) {
	size_t capacity = 8;
	while (capacity < len + 1) {
		capacity *= 2;
	}

	char *new_buffer = malloc(capacity);
	memcpy(new_buffer, buffer, len);
	new_buffer[len] = '\0';

	return (DString) {
		.buffer   = new_buffer,
		.len      = len, 
		.capacity = capacity
	};
}

void dstr_destroy(DString *str) {
	free(str->buffer);
	str->buffer   = NULL;
	str->len      = 0;
	str->capacity = 0;
}

/* Capacity */
size_t dstr_len(const DString *str) {
	return str->len;
}

size_t dstr_capacity(const DString *str) {
	return str->capacity;
}

bool dstr_empty(const DString *str) {
	return str->len == 0;
}

void dstr_clear(DString *str) {
	str->len       = 0;
	str->buffer[0] = '\0';
}

void dstr_reserve(DString *str, size_t capacity) {
	if (capacity <= str->capacity) return;
	str->buffer = realloc(str->buffer, capacity);
	str->capacity = capacity;
}

void dstr_resize(DString *str, size_t new_len) {
	if (new_len + 1 > str->capacity) {
		size_t new_cap = str->capacity == 0 ? 8 : str->capacity;
		while (new_cap < new_len + 1) {
			new_cap *= 2;
		}
		dstr_reserve(str, new_cap);
	}
	str->len           = new_len;
	str->buffer[new_len] = '\0';
}

void dstr_shrink_to_fit(DString *str) {
	str->capacity = str->len + 1;
	str->buffer = realloc(str->buffer, str->capacity);
}

/* Element Acccess */
char dstr_at(const DString *str, size_t index) {
	return str->buffer[index];
}

char dstr_front(const DString *str) {
	return str->buffer[0];
}

char dstr_back(const DString *str) {
	return str->buffer[str->len - 1];
}

char *dstr_data(DString *str) {
	return str->buffer;
}

const char *dstr_cstr(const DString *str) {
	return str->buffer;
}

/* Assignment */
void dstr_copy(DString *dest, const DString *src) {
	while (dest->capacity < src->len + 1) {
		dest->capacity = dest->capacity == 0 ? 8 : dest->capacity * 2;
		dest->buffer    = realloc(dest->buffer, dest->capacity);
	}

	dest->len = src->len;
	memcpy(dest->buffer, src->buffer, dest->len + 1);
}

void dstr_copy_cstr(DString *dest, const char *src) {
	size_t len = strlen(src);

	while (dest->capacity < len + 1) {
		dest->capacity = dest->capacity == 0 ? 8 : dest->capacity * 2;
		dest->buffer    = realloc(dest->buffer, dest->capacity);
	}

	memcpy(dest->buffer, src, len + 1);
	dest->len = len;
}

void dstr_swap(DString *a, DString *b) {
	DString temp = *a;
	*a           = *b;
	*b           = temp;
}

/* Comparison */
int dstr_cmp(const DString *a, const DString *b) {
	size_t min = (a->len < b->len) ? a->len : b->len;

	for (size_t i = 0; i < min; i++) {
		if (a->buffer[i] < b->buffer[i]) return -1;
		if (a->buffer[i] > b->buffer[i]) return  1;
	}

	if (a->len < b->len) return -1;
	if (a->len > b->len) return  1;

	return 0;
}

int dstr_cmp_cstr(const DString *a, const char *b) {
	size_t b_len = strlen(b);
	size_t min = (a->len < b_len) ? a->len : b_len;

	for (size_t i = 0; i < min; i++) {
		if (a->buffer[i] < b[i]) return -1;
		if (a->buffer[i] > b[i]) return  1;
	}

	if (a->len < b_len) return -1;
	if (a->len > b_len) return  1;

	return 0;
}

bool dstr_equal(const DString *a, const DString *b) {
	return a->len == b->len &&
	       memcmp(a->buffer, b->buffer, a->len) == 0;
}

bool dstr_equal_cstr(const DString *a, const char *b) {
	return a->len == strlen(b) &&
	       memcmp(a->buffer, b, a->len) == 0;
}

/* Append */
void dstr_append(DString *dest, const DString *src) {
	size_t req = dest->len + src->len + 1;
	if (req > dest->capacity) {
		size_t new_cap = dest->capacity == 0 ? 8 : dest->capacity;
		while (new_cap < req) {
			new_cap *= 2;
		}
		dstr_reserve(dest, new_cap);
	}

	memcpy(dest->buffer + dest->len, src->buffer, src->len + 1);
	dest->len += src->len;
}

void dstr_append_cstr(DString *dest, const char *src) {
	size_t src_len = strlen(src);

	size_t req = dest->len + src_len + 1;
	if (req > dest->capacity) {
		size_t new_cap = dest->capacity == 0 ? 8 : dest->capacity;
		while (new_cap < req) {
			new_cap *= 2;
		}

		dstr_reserve(dest, new_cap);
	}

	memcpy(dest->buffer + dest->len, src, src_len + 1);
	dest->len += src_len;
}

void dstr_push_back(DString *str, char ch) {
	if (str->len + 2 > str->capacity) {
		size_t new_cap = str->capacity == 0 ? 8 : str->capacity * 2;
		dstr_reserve(str, new_cap);
	}

	str->buffer[str->len] = ch;
	str->len++;
	str->buffer[str->len] = '\0';
}

/* Insert / Erase */
void dstr_insert(DString *str, size_t pos, const DString *src) {
	if (pos > str->len) pos = str->len;

	size_t req = str->len + src->len + 1;
	if (req > str->capacity) {
		size_t new_cap = str->capacity == 0 ? 8 : str->capacity;
		while (new_cap < req) {
			new_cap *= 2;
		}

		dstr_reserve(str, new_cap);
	}

	memmove(str->buffer + pos + src->len, str->buffer + pos, str->len - pos + 1);
	memcpy(str->buffer + pos, src->buffer, src->len);
	str->len += src->len;
}

void dstr_insert_cstr(DString *str, size_t pos, const char *src) {
	if (pos > str->len) pos = str->len;

	size_t src_len = strlen(src);
	size_t req = str->len + src_len + 1;
	if (req > str->capacity) {
		size_t new_cap = str->capacity == 0 ? 8 : str->capacity;
		while (new_cap < req) {
			new_cap *= 2;
		}
		dstr_reserve(str, new_cap);
	}

	memmove(str->buffer + pos + src_len, str->buffer + pos, str->len - pos + 1);
	memcpy(str->buffer + pos, src, src_len);
	str->len += src_len;
}

void dstr_insert_char(DString *str, size_t pos, char ch) {
	if (pos > str->len) pos = str->len;

	if (str->len + 2 > str->capacity) {
		size_t new_cap = str->capacity == 0 ? 8 : str->capacity * 2;
		dstr_reserve(str, new_cap);
	}

	memmove(str->buffer + pos + 1, str->buffer + pos, str->len - pos + 1);
	str->buffer[pos] = ch;
	str->len++;
}

void dstr_erase(DString *str, size_t pos, size_t len) {
	if (pos >= str->len) return;

	if (pos + len > str->len) len = str->len - pos;
	memmove(str->buffer + pos, str->buffer + pos + len, str->len - pos - len + 1);
	str->len -= len;
}

void dstr_pop_back(DString *str) {
	if (str->len == 0) return;

	str->len--;
	str->buffer[str->len] = '\0';
}

/* Searching */
int dstr_find(const DString *str, const DString *needle) {
	if (needle->len == 0) return 0;
	if (needle->len > str->len) return -1;
	
	char *pos = strstr(str->buffer, needle->buffer);
	if (pos == NULL) return -1;

	return (int)(pos - str->buffer);
}

int dstr_find_cstr(const DString *str, const char *needle) {
	if (needle[0] == '\0') return 0;

	char *pos = strstr(str->buffer, needle);
	if (pos == NULL) return -1;

	return (int)(pos - str->buffer);
}

int dstr_rfind(const DString *str, const DString *needle) {
	if (needle->len == 0) return (int)str->len;
	if (needle->len > str->len) return -1;

	for (size_t i = str->len - needle->len + 1; i > 0; i--) {
		if (memcmp(str->buffer + i - 1, needle->buffer, needle->len) == 0) {
			return (int)(i - 1);
		}
	}

	return -1;
}

bool dstr_contains(const DString *str, const DString *needle) {
	return dstr_find(str, needle) != -1;
}

bool dstr_starts_with(const DString *str, const DString *prefix) {
	if (prefix->len > str->len) return false;

	return memcmp(str->buffer, prefix->buffer, prefix->len) == 0;
}

bool dstr_ends_with(const DString *str, const DString *suffix) {
	if (suffix->len > str->len) return false;

	return memcmp(str->buffer + (str->len - suffix->len), suffix->buffer, suffix->len) == 0;
}

/* Substrings */
DString dstr_substr(const DString *str, size_t pos, size_t len) {
	if (pos >= str->len) return dstr_create();
	if (pos + len > str->len) len = str->len - pos;
	return dstr_from_buffer(str->buffer + pos, len);
}

/* Replacement */
void dstr_replace(DString *str, const DString *old, const DString *replacement) {
	if (old->len == 0) return;

	int idx = dstr_find(str, old);
	while (idx != -1) {
		dstr_erase(str, (size_t)idx, old->len);
		dstr_insert(str, (size_t)idx, replacement);
		idx = dstr_find_cstr(str, old->buffer);
	}
}

/* Utilities */
void dstr_trim(DString *str) {
	dstr_trim_left(str);
	dstr_trim_right(str);
}

void dstr_trim_left(DString *str) {
	size_t start = 0;
	while (start < str->len && (str->buffer[start] == ' ' || str->buffer[start] == '\t' || str->buffer[start] == '\n' || str->buffer[start] == '\r')) {
		start++;
	}

	if (start > 0) {
		memmove(str->buffer, str->buffer + start, str->len - start + 1);
		str->len -= start;
	}
}

void dstr_trim_right(DString *str) {
	while (str->len > 0 && (str->buffer[str->len - 1] == ' ' || str->buffer[str->len - 1] == '\t' || str->buffer[str->len - 1] == '\n' || str->buffer[str->len - 1] == '\r')) {
		str->len--;
	}

	str->buffer[str->len] = '\0';
}

void dstr_reverse(DString *str) {
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
}

void dstr_to_upper(DString *str) {
	for (size_t i = 0; i < str->len; i++) {
		if (str->buffer[i] >= 'a' && str->buffer[i] <= 'z') {
			str->buffer[i] -= 32;
		}
	}
}

void dstr_to_lower(DString *str) {
	for (size_t i = 0; i < str->len; i++) {
		if (str->buffer[i] >= 'A' && str->buffer[i] <= 'Z') {
			str->buffer[i] += 32;
		}
	}
}

size_t dstr_count(const DString *str, char ch) {
	size_t count = 0;
	for (size_t i = 0; i < str->len; i++) {
		if (str->buffer[i] == ch)
			count++;
	}

	return count;
}