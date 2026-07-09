#include <stdio.h>
#include "dstring.h"

void process_string_safely(DString *str) {
    // Intentional out of bounds configuration attempt
    char invalid_ch = dstr_at(str, 9999);
    
    if (dstr_get_error_code() != 0) { // Check for non-success
        fprintf(stderr, "[ERROR CAUGHT]: Operation failed. Reason: %s\n", dstr_get_error_string());
        
        // Recover cleanly tracking SDL-like structure
        dstr_clear_error_code();
    } else {
        printf("Character found: %c\n", invalid_ch);
    }
}

int main(void) {
    printf("--- Error Safety API Example ---\n");

    // Error Scenario A: Null defense
    printf("1. Triggering null defense pointer loop:\n");
    dstr_clear(NULL);
    if (dstr_get_error_code() != 0) {
        fprintf(stderr, "[ERROR CAUGHT]: Reason: %s\n", dstr_get_error_string());
    }

    // Error Scenario B: Index tracking boundaries
    printf("\n2. Passing active allocation to unsafe reader:\n");
    DString local_str = dstr_from_cstr("Data");
    process_string_safely(&local_str);

    // Verify operational status is reset to DSTR_SUCCESS seamlessly
    size_t verified_len = dstr_len(&local_str);
    printf("\nStatus checking return code after clean action: '%s' (Length: %zu)\n", 
           dstr_get_error_string(), verified_len);

    dstr_destroy(&local_str);
    return 0;
}