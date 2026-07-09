#include <stdio.h>
#include "dstring.h"

int main(void) {
    printf("--- Utility Transformations Example ---\n");

    // 1. Whitespace handling
    DString padded = dstr_from_cstr(" \t  \n  Clean text input \r\n ");
    printf("Raw text:    \"%s\"\n", dstr_cstr(&padded));
    
    dstr_trim(&padded);
    printf("Trimmed:     \"%s\"\n", dstr_cstr(&padded));

    // 2. Case switching
    dstr_to_upper(&padded);
    printf("Uppercase:   \"%s\"\n", dstr_cstr(&padded));
    
    dstr_to_lower(&padded);
    printf("Lowercase:   \"%s\"\n", dstr_cstr(&padded));

    // 3. Frequency monitoring and character lookups
    size_t e_count = dstr_count(&padded, 'e');
    printf("Occurrences of 'e': %zu\n", e_count);

    // 4. Substring slicing
    DString sliced = dstr_substr(&padded, 6, 4); // grabs "text"
    printf("Sliced substring (pos 6, len 4): \"%s\"\n", dstr_cstr(&sliced));

    dstr_destroy(&padded);
    dstr_destroy(&sliced);
    return 0;
}