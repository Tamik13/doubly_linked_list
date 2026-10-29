#include <assert.h>
#include <math.h>

#include "../include/DLlist.h"

const list_elem_t POISON = NAN;

error_code_e list_create(list_s* const list, size_t size) {
    assert(list != NULL); // TODO: проверить не создан ли уже
    assert(size != 0);

    list->elem_ind = 0;
    list->capacity = size;

    list->data = (list_elem_t*)calloc(size, sizeof(list_elem_t));
    if (list->data == NULL) {
        LOG_ERROR("ERROR DURING CALLOC");
        return ALLOCATION_ERROR;
    }

    list->next = (ssize_t*)calloc(size, sizeof(list_elem_t));
    if (list->next == NULL) {
        LOG_ERROR("ERROR DURING CALLOC");
        return ALLOCATION_ERROR;
    }

    list->data[0] = POISON;

    for (size_t ind = 1; ind < size; ind++) {
        list->data[ind] = POISON;
        list->next[ind] = -1;
    }

    LOG_OK("list created successfully");
    return SUCCESS;
}


error_code_e list_destroy(list_s* const list) {
    assert(list       != NULL);
    assert(list->data != NULL);
    assert(list->next != NULL);

    if (is_readble_ptr(list) != SUCCESS || is_readble_ptr(list->data) != SUCCESS || is_readble_ptr(list->next) != SUCCESS) {
        LOG_ERROR("LIST CANT BE DESTROY, CORRUPTED POINTER list = %p list->data = %p list->next = %p", list, list->data, list->next);
        return INCORRECT_PARAM;
    }

    free(list->data);
    free(list->next);

    list->data     = NULL;
    list->next     = NULL;
    list->elem_ind = -1;
    list->capacity = 0;

    LOG_OK("list destroy successfully");
    return SUCCESS;
}


error_code_e list_next(list_s* const list) {
     // TODO: list_verify
    assert(list       != NULL);
    assert(list->data != NULL);
    assert(list->next != NULL);

    SAFE_CALL(list_verify(list))

    list->elem_ind = list->next[list->elem_ind];

    LOG_OK("list next successfully elem_ind = %zd", list->elem_ind);
    return SUCCESS;
}


error_code_e list_get(const list_s* const list, list_elem_t* value) {
    assert(value      != NULL); // TODO list_verify
    assert(list       != NULL);
    assert(list->data != NULL);
    assert(list->next != NULL);

    SAFE_CALL(list_verify(list))

    *value = list->data[list->elem_ind];

    LOG_OK("list get successfully get_value = " LST_MDF, *value);
    return SUCCESS;
}


error_code_e list_add(list_s* const list, list_elem_t value) {
    assert(list       != NULL); // TODO realloc
    assert(list->data != NULL);
    assert(list->next != NULL);

    SAFE_CALL(list_verify(list))

    ssize_t free_ind = find_free_ind(list);

    if (free_ind == -1) {
        LOG_ERROR("ERROR DURING FIND FREE IND");
        return LIST_OVERFLOW;
    }


    list->data[free_ind]       = value;
    list->next[free_ind]       = list->next[list->elem_ind];
    list->next[list->elem_ind] = free_ind;

    LOG_OK("list add successfully add_value = " LST_MDF, value);
    return SUCCESS;
}


error_code_e list_del(list_s* const list) {
    assert(list       != NULL);
    assert(list->data != NULL);
    assert(list->next != NULL);

    if (list->elem_ind <= 0) {
        LOG_ERROR("LIST UNDERFLOW");
        return LIST_UNDERFLOW;
    }

    ssize_t prev_ind = -1;

    for (ssize_t ind = 0; ind < (ssize_t)list->capacity; ind++) {
        if (list->next[ind] == list->elem_ind) {
            prev_ind = ind;
        }
    }

    if (prev_ind == -1) {
        LOG_ERROR("CANT FIND PREV_IND");
        return INCORRECT_PARAM;
    }

    list->data[list->elem_ind] = POISON;
    list->next[list->elem_ind] = -1;
    list->elem_ind             = prev_ind;

    LOG_OK("list del successfully");
    return SUCCESS;
}


error_code_e list_verify(const list_s* const list) {
    if (list == NULL) { // TODO битовые поля
        LOG_ERROR("list = NULL");
        return NULL_PARAM;
    }
    if (is_readble_ptr(list) != SUCCESS) {
        LOG_ERROR("list ptr cant be read");
        return SEG_FAULT;
    }

    if (list->data == NULL) {
        LOG_ERROR("list->data = NULL");
        return NULL_PARAM;
    }
    if (is_readble_ptr(list->data) != SUCCESS) {
        LOG_ERROR("list->data ptr cant be read");
        return SEG_FAULT;
    }

    if (list->next == NULL) {
        LOG_ERROR("list = NULL");
        return NULL_PARAM;
    }
    if (is_readble_ptr(list->next) != SUCCESS) {
        LOG_ERROR("list ptr cant be read");
        return SEG_FAULT;
    }

    if (!isnan(list->data[0])) { // При замене POISON поменять!!!
        LOG_ERROR("list->data[0] != POISON");
        return CANARY_IS_DEAD;
    }

    return SUCCESS;
}


ssize_t find_free_ind(const list_s* const list) {
    assert(list       != NULL);
    assert(list->data != NULL);
    assert(list->next != NULL);

    SAFE_CALL(list_verify(list))

    for (ssize_t ind = 1; ind < (ssize_t)list->capacity; ind++) {
        if (list->next[ind] == -1) {
            LOG_OK("find_free_ind successfully ind = %zd", ind);
            return ind;
        }
    }

    LOG_ERROR("CANT FIND FREE IND ind = %zd", -1);
    return -1;
}


void list_dmp(const list_s* const list) {
    assert(list       != NULL);
    assert(list->data != NULL);
    assert(list->next != NULL); // TODO list_ptr verify

    LOG("list %p {", list);

    if (is_readble_ptr(list) != SUCCESS) {
        LOG_ERROR("list cant be read");
        LOG("}");
        return;
    }


    LOG("\tdata %p {", list->data);

    if (is_readble_ptr(list->data) == SUCCESS) {
        for (size_t ind = 0; ind < list->capacity; ind++) {
            LOG("\t\t[%zu] = " LST_MDF, ind, list->data[ind]);
        }
    } else {
        LOG_ERROR("\t\tlist->data cant be read");
    }

    LOG("\t}");
    LOG("\tnext %p {", list->next);

    if (is_readble_ptr(list->next) == SUCCESS) {
        for (size_t ind = 0; ind < list->capacity; ind++) {
            LOG("\t\t[%zu] = %zd", ind, list->next[ind]);
        }
    } else {
        LOG_ERROR("\t\tlist->next cant be read");
    }

    LOG("\t}");
    LOG("\telem_ind = %zd", list->elem_ind);
    LOG("\ttail     = %zu", list->next[0]);
    LOG("\tcapacity = %zu", list->capacity);
    LOG("}");

    return;
}

