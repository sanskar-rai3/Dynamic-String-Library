#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "dstring.h"

int main(void) {
    printf("Starting DString verification tests...\n\n");

    /* 1. Construction / Destruction & Capacity */
    DString s1 = dstr_create();
    assert(dstr_len(&s1) == 0);
    assert(dstr_capacity(&s1) >= 8);
    assert(dstr_empty(&s1) == true);

    DString s2 = dstr_from_cstr("Hello, World!");
    assert(dstr_len(&s2) == 13);
    assert(strcmp(dstr_cstr(&s2), "Hello, World!") == 0);

    DString s3 = dstr_from_buffer("Embedded\0Null", 13);
    assert(dstr_len(&s3) == 13);

    dstr_clear(&s1);
    assert(dstr_len(&s1) == 0);

    dstr_reserve(&s1, 50);
    assert(dstr_capacity(&s1) >= 50);

    dstr_resize(&s1, 10);
    assert(dstr_len(&s1) == 10);

    dstr_shrink_to_fit(&s1);
    assert(dstr_capacity(&s1) == 11);

    /* 2. Element Access */
    DString acc = dstr_from_cstr("ABC");
    assert(dstr_at(&acc, 1) == 'B');
    assert(dstr_front(&acc) == 'A');
    assert(dstr_back(&acc) == 'C');
    assert(dstr_data(&acc) != NULL);
    assert(strcmp(dstr_cstr(&acc), "ABC") == 0);

    /* 3. Assignment */
    DString dest = dstr_create();
    dstr_copy(&dest, &acc);
    assert(dstr_equal(&dest, &acc));

    dstr_copy_cstr(&dest, "XYZ");
    assert(strcmp(dstr_cstr(&dest), "XYZ") == 0);

    dstr_swap(&acc, &dest);
    assert(strcmp(dstr_cstr(&acc), "XYZ") == 0);
    assert(strcmp(dstr_cstr(&dest), "ABC") == 0);

    /* 4. Comparison */
    DString cmp1 = dstr_from_cstr("apple");
    DString cmp2 = dstr_from_cstr("banana");
    assert(dstr_cmp(&cmp1, &cmp2) < 0);
    assert(dstr_cmp_cstr(&cmp1, "apple") == 0);
    assert(dstr_equal(&cmp1, &cmp1));
    assert(dstr_equal_cstr(&cmp1, "apple"));

    /* 5. Append */
    DString app = dstr_from_cstr("Hello");
    DString tail = dstr_from_cstr(" World");
    dstr_append(&app, &tail);
    assert(strcmp(dstr_cstr(&app), "Hello World") == 0);

    dstr_append_cstr(&app, "!");
    assert(strcmp(dstr_cstr(&app), "Hello World!") == 0);

    dstr_push_back(&app, '?');
    assert(strcmp(dstr_cstr(&app), "Hello World!?") == 0);

    /* 6. Insert / Erase */
    DString ie = dstr_from_cstr("Hllo");
    DString ins = dstr_from_cstr("e");
    dstr_insert(&ie, 1, &ins);
    assert(strcmp(dstr_cstr(&ie), "Hello") == 0);

    dstr_insert_cstr(&ie, 5, " World");
    assert(strcmp(dstr_cstr(&ie), "Hello World") == 0);

    dstr_insert_char(&ie, 11, '!');
    assert(strcmp(dstr_cstr(&ie), "Hello World!") == 0);

    dstr_erase(&ie, 5, 7);
    assert(strcmp(dstr_cstr(&ie), "Hello") == 0);

    dstr_pop_back(&ie);
    assert(strcmp(dstr_cstr(&ie), "Hell") == 0);

    /* 7. Searching */
    DString src = dstr_from_cstr("banana");
    DString ndl = dstr_from_cstr("an");
    assert(dstr_find(&src, &ndl) == 1);
    assert(dstr_find_cstr(&src, "na") == 2);
    assert(dstr_rfind(&src, &ndl) == 3);
    assert(dstr_contains(&src, &ndl) == true);

    DString pref = dstr_from_cstr("ba");
    DString suff = dstr_from_cstr("na");
    assert(dstr_starts_with(&src, &pref) == true);
    assert(dstr_ends_with(&src, &suff) == true);

    /* 8. Substrings */
    DString sub = dstr_substr(&src, 2, 2);
    assert(strcmp(dstr_cstr(&sub), "na") == 0);

    /* 9. Replacement */
    DString rep = dstr_from_cstr("cats and dogs");
    DString old_str = dstr_from_cstr("cats");
    DString new_str = dstr_from_cstr("birds");
    dstr_replace(&rep, &old_str, &new_str);
    assert(strcmp(dstr_cstr(&rep), "birds and dogs") == 0);

    /* 10. Utilities */
    DString trm = dstr_from_cstr("  \t text \n  ");
    dstr_trim(&trm);
    assert(strcmp(dstr_cstr(&trm), "text") == 0);

    DString rev = dstr_from_cstr("live");
    dstr_reverse(&rev);
    assert(strcmp(dstr_cstr(&rev), "evil") == 0);

    DString case_str = dstr_from_cstr("MiXed");
    dstr_to_upper(&case_str);
    assert(strcmp(dstr_cstr(&case_str), "MIXED") == 0);
    dstr_to_lower(&case_str);
    assert(strcmp(dstr_cstr(&case_str), "mixed") == 0);

    assert(dstr_count(&case_str, 'm') == 1);

    /* Clean up all allocated dynamic strings */
    dstr_destroy(&s1);
    dstr_destroy(&s2);
    dstr_destroy(&s3);
    dstr_destroy(&acc);
    dstr_destroy(&dest);
    dstr_destroy(&cmp1);
    dstr_destroy(&cmp2);
    dstr_destroy(&app);
    dstr_destroy(&tail);
    dstr_destroy(&ie);
    dstr_destroy(&ins);
    dstr_destroy(&src);
    dstr_destroy(&ndl);
    dstr_destroy(&pref);
    dstr_destroy(&suff);
    dstr_destroy(&sub);
    dstr_destroy(&rep);
    dstr_destroy(&old_str);
    dstr_destroy(&new_str);
    dstr_destroy(&trm);
    dstr_destroy(&rev);
    dstr_destroy(&case_str);

    printf("All DString function verifications completed successfully!\n");
    
    return 0;
}