#ifndef _DLLIST_H
#define _DLLIST_H

#include "../include/debugging.h"

typedef double list_elem_t;
#define LST_MDF "%lg"

struct list_s {
    list_elem_t* data;
    ssize_t*     next;
    ssize_t      elem_ind;
    size_t       capacity;
};

error_code_e list_create (      list_s* const list, size_t size);
error_code_e list_destroy(      list_s* const list);
error_code_e list_next   (      list_s* const list);
error_code_e list_get    (const list_s* const list, list_elem_t* value);
error_code_e list_add    (      list_s* const list, list_elem_t  value);
error_code_e list_del    (      list_s* const list);
error_code_e list_verify (const list_s* const list);

ssize_t find_free_ind(const list_s* const list);
void    list_dmp     (const list_s* const list);

#endif
