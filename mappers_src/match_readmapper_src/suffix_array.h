
#ifndef SUFFIX_ARRAY_H
#define SUFFIX_ARRAY_H

#include "match.h"
#include <stddef.h>
#include <assert.h>

struct suffix_array {
    // the suffix array owns this, so copy it if you want to keep it.
    // it will be freed when the suffix array is freed.
    char *string;
    // length of the array
    size_t length;
    // the actual suffix array
    size_t *array;
};

struct suffix_array *qsort_sa_construction(char *string);
void delete_suffix_array(struct suffix_array *sa);

size_t lower_bound_search(struct suffix_array *sa, const char *key);

void suffix_array_bsearch_match(const char *text, size_t n,
                                const char *pattern, size_t m,
                                match_callback_func callback,
                                void *callback_data);

#endif
