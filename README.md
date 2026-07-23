# dstring

A small, dependency-free dynamic string library for C, in the spirit of C++'s `std::string` — growable buffers, explicit length tracking, and a consistent error-reporting model, all in plain C11.

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

## Features

- Automatic buffer growth (geometric doubling) for append, insert, and resize operations
- Explicit length tracking (`len`), so most operations that don't rely on `strstr`/`strlen` internally support embedded `'\0'` bytes
- Always null-terminated, so a `DString` can be passed anywhere a `const char *` is expected via `dstr_cstr()`
- A single global error code (`DStringError`), set by every call, checkable without out-parameters
- Zero dependencies beyond the C standard library (`stdlib.h`, `string.h`, `stddef.h`, `stdbool.h`)

## Table of contents

- [Building](#building)
- [The `DString` type](#the-dstring-type)
- [Error handling](#error-handling)
- [API reference](#api-reference)
- [Notes and known limitations](#notes-and-known-limitations)
- [License](#license)

## Building

There's no build system bundled — drop `dstring.h` and `dstring.c` into your project and compile them alongside your own sources:

```sh
gcc -std=c11 -Wall -Wextra -c dstring.c -o dstring.o
```

Then link `dstring.o` into your project, or just add `dstring.c` directly to your existing build (CMake, Make, etc.).

Example programs demonstrating the API are available in the `demo/` directory.

## The `DString` type

```c
typedef struct {
    char   *buffer;
    size_t  len;
    size_t  capacity;
} DString;
```

`DString` is a plain value type. It's returned by value from constructors, and its fields are safe to read directly, but:

- **The buffer must be released exactly once**, via `dstr_destroy()`.
- **`=` copies the pointer, not the data.** Assigning one `DString` to another (`DString b = a;`) gives you two structs pointing at the same heap buffer — destroying one and then using the other is a use-after-free, and destroying both double-frees. Use `dstr_copy()` (or `dstr_copy_cstr()`) whenever you need an independent, owned copy.

## Error handling

Every function sets a **single global error code**, retrievable at any time:

```c
DStringError dstr_get_error_code(void);
const char  *dstr_get_error_string(void);
void         dstr_clear_error_code(void);
```

| Code | Meaning |
|---|---|
| `DSTR_SUCCESS` | No error |
| `DSTR_ERROR_ALLOC_FAILED` | `malloc`/`realloc` failed |
| `DSTR_ERROR_OUT_OF_BOUNDS` | Index or position out of range |
| `DSTR_ERROR_NULL_ARGUMENT` | A required pointer argument was `NULL` |
| `DSTR_ERROR_INVALID_SIZE` | Requested size/capacity is invalid |
| `DSTR_ERROR_NOT_FOUND` | Search target wasn't found |

```c
dstr_find_cstr(&s, "xyz");
if (dstr_get_error_code() == DSTR_ERROR_NOT_FOUND) {
    // handle it
}
```

> [!IMPORTANT]
> **Check the error code immediately after the call that can fail — before calling any other `dstr_*` function.** Nearly every function in this library, including read-only accessors like `dstr_len()` and `dstr_empty()`, resets the global error code to `DSTR_SUCCESS` on their own success path. If anything else runs between the risky call and your check, the failure you're looking for can be silently overwritten:
>
> ```c
> dstr_append_cstr(&s, big_string);   // suppose this fails (alloc)
> size_t n = dstr_len(&s);            // this call resets the error code to SUCCESS
> if (dstr_get_error_code() == DSTR_ERROR_ALLOC_FAILED) {
>     // never reached — the failure was already overwritten
> }
> ```
>
> This is the same class of hazard as libc's `errno`: treat the global as valid for exactly one check, right after the call that set it.

The error code is **global, not per-`DString`**, and this library assumes a **single-threaded** program — it is not thread-safe as written.

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
| `void dstr_replace(DString *str, const DString *old, const DString *replacement)` | Replace all non-overlapping occurrences of `old` with `replacement`, left to right |

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

- **`dstr_find`/`dstr_find_cstr`/`dstr_replace` are `strstr`-based.** A `needle`/`old` containing an embedded `'\0'` byte will be truncated at the first NUL rather than matched against its full tracked length. `dstr_rfind` doesn't share this limitation, since it compares with `memcmp` over the full tracked length.
- **`int`-based indices.** `dstr_find`, `dstr_find_cstr`, and `dstr_rfind` return `int`, so results on strings larger than `INT_MAX` bytes aren't representable. Not a concern for typical use, but worth knowing if you're working with very large buffers.
- **`dstr_at`/`dstr_front`/`dstr_back` return `'\0'` on error**, which is indistinguishable from a legitimately stored `'\0'` character — check `dstr_get_error_code()` if you need to tell the two apart.
- **ASCII-only case conversion and whitespace trimming.** No locale or Unicode awareness; multi-byte UTF-8 sequences pass through `dstr_to_upper`/`dstr_to_lower` unmodified but non-ASCII whitespace (e.g. U+00A0) is not trimmed.
- **The global error code is a single point of failure for diagnostics.** Because it's overwritten by nearly every call, including read-only accessors, don't rely on it surviving more than one call after the operation you're checking. See [Error handling](#error-handling) above.

## License

This project is licensed under the MIT License.

Full license text is available in the `LICENSE` file.