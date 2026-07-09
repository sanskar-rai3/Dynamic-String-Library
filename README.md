# dstring

A small, dependency-free dynamic string library for C, in the spirit of C++'s `std::string` — growable buffers, length tracking, and a consistent error-reporting model, all in plain C11.

## Features

- Automatic buffer growth (geometric doubling) for append, insert, and resize operations
- Explicit length tracking (`len`), so embedded `'\0'` bytes are supported by most operations that don't rely on `strstr`/`strlen` internally
- Always null-terminated, so a `DString` can be passed anywhere a `const char *` is expected via `dstr_cstr()`
- A single global error code (`DStringError`) set by every call, so you can check what went wrong without out-parameters
- No dependencies beyond the C standard library (`stdlib.h`, `string.h`, `stddef.h`, `stdbool.h`)

## Building

There's no build system bundled — drop `dstring.h` and `dstring.c` into your project and compile them alongside your own sources:

```sh
gcc -std=c11 -Wall -Wextra -c dstring.c -o dstring.o
```

Then link `dstring.o` into your project as usual, or just add `dstring.c` to your existing build.

## Quick example

```c
#include <stdio.h>
#include "dstring.h"

int main(void) {
    DString s = dstr_from_cstr("  Hello, World!  ");

    dstr_trim(&s);
    dstr_to_lower(&s);

    printf("%s\n", dstr_cstr(&s)); // "hello, world!"

    dstr_destroy(&s);
    return 0;
}
```

## The `DString` type

```c
typedef struct {
    char   *buffer;
    size_t  len;
    size_t  capacity;
} DString;
```

`DString` is a plain value type — it's returned by value from constructors and can be copied by assignment, but the `buffer` it owns must be released exactly once with `dstr_destroy()`. Copying a `DString` with `=` copies the pointer, not the data — use `dstr_copy()` for a deep copy.

Example programs demonstrating the library's API are available in the `demo/` directory.


## Error handling

Every function sets a global error code, retrievable via:

```c
DStringError dstr_get_error_code(void);
const char  *dstr_get_error_string(void);
void         dstr_clear_error_code(void);
```

Possible codes:

| Code | Meaning |
|---|---|
| `DSTR_SUCCESS` | No error |
| `DSTR_ERROR_ALLOC_FAILED` | `malloc`/`realloc` failed |
| `DSTR_ERROR_OUT_OF_BOUNDS` | Index or position out of range |
| `DSTR_ERROR_NULL_ARGUMENT` | A required pointer argument was `NULL` |
| `DSTR_ERROR_INVALID_SIZE` | Requested size/capacity is invalid |
| `DSTR_ERROR_NOT_FOUND` | Search target wasn't found |

The error code is **global**, not per-`DString`, and this library assumes a **single-threaded** program — it is not thread-safe as-is.

```c
dstr_find_cstr(&s, "xyz");
if (dstr_get_error_code() == DSTR_ERROR_NOT_FOUND) {
    // handle it
}
```

## API reference

### Construction / destruction

| Function | Description |
|---|---|
| `DString dstr_create(void)` | Empty string with default capacity |
| `DString dstr_from_cstr(const char *str)` | Build from a null-terminated C string |
| `DString dstr_from_buffer(const char *buffer, size_t len)` | Build from raw bytes + explicit length |
| `void dstr_destroy(DString *str)` | Free the buffer; resets the struct to empty |

### Capacity

| Function | Description |
|---|---|
| `size_t dstr_len(const DString *str)` | Current length |
| `size_t dstr_capacity(const DString *str)` | Current allocated capacity |
| `bool dstr_empty(const DString *str)` | `true` if length is 0 |
| `void dstr_clear(DString *str)` | Truncate to length 0 (keeps capacity) |
| `void dstr_reserve(DString *str, size_t capacity)` | Ensure at least this much capacity |
| `void dstr_resize(DString *str, size_t new_len)` | Grow or shrink to `new_len`; newly exposed bytes on growth are zero-filled |
| `void dstr_shrink_to_fit(DString *str)` | Reallocate down to exactly fit the current length |

### Element access

| Function | Description |
|---|---|
| `char dstr_at(const DString *str, size_t index)` | Bounds-checked character access |
| `char dstr_front(const DString *str)` | First character |
| `char dstr_back(const DString *str)` | Last character |
| `char *dstr_data(DString *str)` | Mutable pointer to the internal buffer |
| `const char *dstr_cstr(const DString *str)` | Null-terminated C string view |

### Assignment

| Function | Description |
|---|---|
| `void dstr_copy(DString *dest, const DString *src)` | Deep copy `src` into `dest` |
| `void dstr_copy_cstr(DString *dest, const char *src)` | Deep copy a C string into `dest` |
| `void dstr_swap(DString *a, DString *b)` | Swap two strings' internal state |

### Comparison

| Function | Description |
|---|---|
| `int dstr_cmp(const DString *a, const DString *b)` | `strcmp`-style three-way comparison |
| `int dstr_cmp_cstr(const DString *a, const char *b)` | Same, against a C string |
| `bool dstr_equal(const DString *a, const DString *b)` | Equality check |
| `bool dstr_equal_cstr(const DString *a, const char *b)` | Equality check against a C string |

### Appending

| Function | Description |
|---|---|
| `void dstr_append(DString *dest, const DString *src)` | Append another `DString` |
| `void dstr_append_cstr(DString *dest, const char *src)` | Append a C string |
| `void dstr_push_back(DString *str, char ch)` | Append a single character |

### Insert / erase

| Function | Description |
|---|---|
| `void dstr_insert(DString *str, size_t pos, const DString *src)` | Insert a `DString` at `pos` |
| `void dstr_insert_cstr(DString *str, size_t pos, const char *src)` | Insert a C string at `pos` |
| `void dstr_insert_char(DString *str, size_t pos, char ch)` | Insert a single character at `pos` |
| `void dstr_erase(DString *str, size_t pos, size_t len)` | Remove `len` characters starting at `pos` |
| `void dstr_pop_back(DString *str)` | Remove the last character |

### Searching

| Function | Description |
|---|---|
| `int dstr_find(const DString *str, const DString *needle)` | First index of `needle`, or `-1` |
| `int dstr_find_cstr(const DString *str, const char *needle)` | Same, against a C string |
| `int dstr_rfind(const DString *str, const DString *needle)` | Last index of `needle`, or `-1` |
| `bool dstr_contains(const DString *str, const DString *needle)` | Whether `needle` occurs anywhere |
| `bool dstr_starts_with(const DString *str, const DString *prefix)` | Prefix check |
| `bool dstr_ends_with(const DString *str, const DString *suffix)` | Suffix check |

### Substrings & replacement

| Function | Description |
|---|---|
| `DString dstr_substr(const DString *str, size_t pos, size_t len)` | New `DString` copied from a slice |
| `void dstr_replace(DString *str, const DString *old, const DString *replacement)` | Replace all non-overlapping occurrences of `old` with `replacement` |

### Utilities

| Function | Description |
|---|---|
| `void dstr_trim(DString *str)` | Strip leading and trailing whitespace (space, tab, `\n`, `\r`) |
| `void dstr_trim_left(DString *str)` | Strip leading whitespace only |
| `void dstr_trim_right(DString *str)` | Strip trailing whitespace only |
| `void dstr_reverse(DString *str)` | Reverse in place |
| `void dstr_to_upper(DString *str)` | ASCII uppercase in place |
| `void dstr_to_lower(DString *str)` | ASCII lowercase in place |
| `size_t dstr_count(const DString *str, char ch)` | Count occurrences of a character |

## Notes and known limitations

- **Not thread-safe.** The error code is a single global (`g_dstr_last_error`); concurrent use from multiple threads will race.
- **`dstr_find`/`dstr_find_cstr`/`dstr_replace` are `strstr`-based**, so a `needle`/`old` containing an embedded `'\0'` byte will be truncated at the first NUL rather than matched by its full tracked length.
- **`int`-based indices.** `dstr_find`, `dstr_find_cstr`, and `dstr_rfind` return `int`, so results on strings larger than `INT_MAX` bytes aren't representable — not a concern for typical use, but worth knowing.
- **`dstr_at`/`dstr_front`/`dstr_back` return `'\0'` on error** as well as when `'\0'` is a legitimately stored character — check `dstr_get_error_code()` if you need to distinguish the two.
- **ASCII-only case conversion and whitespace trimming** — no locale or Unicode awareness.

## License

This project is licensed under the MIT License.

Full license text is available in the 'LICENSE' file.