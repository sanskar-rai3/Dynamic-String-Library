#include <stdio.h>
#include "dstring.h"

int main(void) {
    printf("--- Basic Manipulation Example ---\n");

    // 1. Initial creation
    DString message = dstr_from_cstr("Hello");
    printf("Initial string: %s (Len: %zu, Cap: %zu)\n", 
           dstr_cstr(&message), dstr_len(&message), dstr_capacity(&message));

    // 2. Modifying content via structural insertion
    dstr_insert_cstr(&message, 5, " Python!");
    printf("Post-insert:    %s\n", dstr_cstr(&message));

    // 3. String replacements
    DString old_word = dstr_from_cstr("Python");
    DString new_word = dstr_from_cstr("C Programming");
    dstr_replace(&message, &old_word, &new_word);
    printf("Post-replace:   %s\n", dstr_cstr(&message));

    // 4. Element modification 
    dstr_pop_back(&message); // Remove '!'
    dstr_push_back(&message, '.'); // Append '.'
    printf("Post-char mod:  %s\n", dstr_cstr(&message));

    // Cleanup memory
    dstr_destroy(&message);
    dstr_destroy(&old_word);
    dstr_destroy(&new_word);

    return 0;
}