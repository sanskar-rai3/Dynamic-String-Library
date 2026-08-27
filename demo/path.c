#include <stdio.h>
#include "dstring.h"

int main(void) {
    printf("--- File Path Builder Example ---\n");

    DString path = dstr_create();

    // Simulating component assembly step-by-step
    dstr_append_cstr(&path, "/usr");
    dstr_append_cstr(&path, "/local");
    dstr_append_cstr(&path, "/bin");
    dstr_push_back(&path, '/');
    dstr_append_cstr(&path, "executable_app");

    printf("Constructed absolute path:\n%s\n", dstr_cstr(&path));
    printf("Current allocation capacity: %zu bytes\n", dstr_capacity(&path));

    // Performance optimization hint: optimizing final footprints
    dstr_shrink_to_fit(&path);
    printf("Capacity optimized after shrink: %zu bytes\n", dstr_capacity(&path));

    // Path verification attributes
    DString suffix = dstr_from_cstr("executable_app");
    if (dstr_ends_with(&path, &suffix)) {
        printf("Verification: Path targets an executable.\n");
    }

    dstr_destroy(&path);
    dstr_destroy(&suffix);
    return 0;
}