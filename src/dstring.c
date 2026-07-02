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

	return (DString) {
		.buffer   = new_buffer,
		.len      = len, 
		.capacity = capacity
	};
}

void dstr_destroy(DString *str) {
	free(str->buffer);
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
	if (capacity > str->capacity) str->capacity = capacity;

	str->buffer = realloc(str->buffer, capacity);
}

void dstr_resize(DString *str, size_t new_len) {
	str->len             = new_len;
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
		dest->buffer    = realloc(dest->buffer, dest->capacity * 2);
		dest->capacity *= 2;
	}

	dest->len = src->len;
	memcpy(dest->buffer, src->buffer, dest->len + 1);
}

void dstr_copy_cstr(DString *dest, const char *src) {
	size_t len = strlen(src);

	while (dest->capacity < len + 1) {
		dest->buffer    = realloc(dest->buffer, dest->capacity * 2);
		dest->capacity *= 2;
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
	size_t min = (a->len < strlen(b)) ? a->len : strlen(b);

	for (size_t i = 0; i < min; i++) {
		if (a->buffer[i] < b[i]) return -1;
		if (a->buffer[i] > b[i]) return  1;
	}

	if (a->len < str(b))    return -1;
	if (a->len > strlen(b)) return  1;

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
