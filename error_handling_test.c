#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "dstring.h"

/* * Assuming these types/functions are declared in dstring.h from your previous inputs:
 * * typedef enum {
 * DSTR_SUCCESS = 0,
 * DSTR_ERROR_ALLOC_FAILED,
 * DSTR_ERROR_OUT_OF_BOUNDS,
 * DSTR_ERROR_NULL_ARGUMENT,
 * DSTR_ERROR_INVALID_SIZE,
 * DSTR_ERROR_NOT_FOUND,
 * DSTR_ERROR_COUNT
 * } DStringError;
 * * void dstr_set_error_code(DStringError code);
 * DStringError dstr_get_error_code(void);
 * const char *dstr_get_error_string(void);
 * void dstr_clear_error_code(void);
 */

int main(void) {
    printf("Starting DString Error Handling Verification Tests...\n\n");

    /* Test 1: Baseline Initial/Clear State */
    dstr_clear_error_code();
    assert(dstr_get_error_code() == DSTR_SUCCESS);
    assert(strcmp(dstr_get_error_string(), "") == 0);
    printf("[PASS] Initial baseline states are verified.\n");

    /* Test 2: Passing NULL Argument to functions */
    DStringError res_null;
    
    // Call function with NULL argument
    dstr_clear(NULL);
    assert(dstr_get_error_code() == DSTR_ERROR_NULL_ARGUMENT);
    assert(strcmp(dstr_get_error_string(), "Null pointer passed as a required argument") == 0);
    printf("[PASS] DSTR_ERROR_NULL_ARGUMENT handled correctly.\n");

    /* Test 3: Index Out of Bounds State */
    DString str = dstr_from_cstr("Hello");
    assert(dstr_get_error_code() == DSTR_SUCCESS);

    // Intentional bad index read via dstr_at
    char invalid_char = dstr_at(&str, 20);
    assert(invalid_char == '\0');
    assert(dstr_get_error_code() == DSTR_ERROR_OUT_OF_BOUNDS);
    assert(strcmp(dstr_get_error_string(), "Index or position out of bounds") == 0);
    printf("[PASS] dstr_at out-of-bounds error verified.\n");

    // Intentional bad index erase
    dstr_clear_error_code();
    dstr_erase(&str, 10, 2);
    assert(dstr_get_error_code() == DSTR_ERROR_OUT_OF_BOUNDS);
    printf("[PASS] dstr_erase out-of-bounds error verified.\n");

    // Intentional pop_back on empty string
    DString empty_str = dstr_create();
    dstr_clear_error_code();
    dstr_pop_back(&empty_str);
    assert(dstr_get_error_code() == DSTR_ERROR_OUT_OF_BOUNDS);
    printf("[PASS] dstr_pop_back on empty string error verified.\n");

    /* Test 4: Target Not Found Error State */
    dstr_clear_error_code();
    DString needle = dstr_from_cstr("World");
    int index = dstr_find(&str, &needle);
    assert(index == -1);
    assert(dstr_get_error_code() == DSTR_ERROR_NOT_FOUND);
    assert(strcmp(dstr_get_error_string(), "Substring or character target not found") == 0);
    printf("[PASS] dstr_find matching target not found error verified.\n");

    /* Test 5: Standard Function Call Resets Error Code to Success */
    // If a function succeeds, it must change g_dstr_last_error back to DSTR_SUCCESS
    assert(dstr_get_error_code() == DSTR_ERROR_NOT_FOUND); // ongoing state
    size_t length = dstr_len(&str);
    assert(length == 5);
    assert(dstr_get_error_code() == DSTR_SUCCESS);
    assert(strcmp(dstr_get_error_string(), "") == 0);
    printf("[PASS] Success reset functionality verified.\n");

    /* Test 6: Manual Clears */
    dstr_set_error_code(DSTR_ERROR_INVALID_SIZE);
    assert(dstr_get_error_code() == DSTR_ERROR_INVALID_SIZE);
    dstr_clear_error_code();
    assert(dstr_get_error_code() == DSTR_SUCCESS);
    printf("[PASS] Manual clear error mechanics verified.\n");

    /* Clean up allocated strings */
    dstr_destroy(&str);
    dstr_destroy(&needle);
    dstr_destroy(&empty_str);

    printf("\nAll DString error framework verifications completed successfully!\n");

    return 0;
}