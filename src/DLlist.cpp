#include <assert.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "../include/DLlist.h"

const list_elem_t POISON = NAN;

error_code_e list_assign(list_s* const list, ssize_t size) {
    assert(list != NULL); // TODO: проверить не создан ли уже

    if (size < 1) {
        LOG_ERROR("INCORRECT SIZE: size = %zu", size);
        return INCORRECT_SIZE;
    }

    size++;

    list->elem_ind = 0;
    list->capacity = size;
    list->free_ind = 1;

    list->data = (list_elem_t*)calloc((size_t)size, sizeof(list_elem_t));
    if (list->data == NULL) {
        LOG_ERROR("ERROR DURING CALLOC");
        return ALLOCATION_ERROR;
    }

    list->next = (ssize_t*)calloc((size_t)size, sizeof(ssize_t));
    if (list->next == NULL) {
        free(list->data);
        LOG_ERROR("ERROR DURING CALLOC");
        return ALLOCATION_ERROR;
    }

    list->prev = (ssize_t*)calloc((size_t)size, sizeof(ssize_t));
    if (list->prev == NULL) {
        free(list->data);
        free(list->next);
        LOG_ERROR("ERROR DURING CALLOC");
        return ALLOCATION_ERROR;
    }

    for (ssize_t ind = 1; ind < size; ind++) {
        list->data[ind] = POISON;
        list->next[ind] = (ssize_t)ind + 1;
        list->prev[ind] = -1;
    }

    list->data[0]        = POISON;
    list->next[size - 1] = -1;

    LOG_OK("list created successfully");
    return SUCCESS;
}


error_code_e list_recalloc(list_s* const list) {
    LOG("start list_destroy");

    assert(list       != NULL);
    assert(list->data != NULL);
    assert(list->next != NULL);
    assert(list->prev != NULL);

    SAFE_CALL(list_verify(list))

    const ssize_t HIGHER_COEF = 2;
    ssize_t new_capacity = list->capacity * HIGHER_COEF;

    list_elem_t* new_data = (list_elem_t*)realloc(list->data, (size_t)new_capacity * sizeof(list_elem_t));
    if (new_data == NULL) {
        LOG_ERROR("ERROR DURING RECALLOC new_data");
        return ALLOCATION_ERROR;
    }

    for (ssize_t ind = list->capacity; ind < new_capacity; ind++) {
        new_data[ind] = POISON;
    }

    ssize_t* new_next = (ssize_t*)realloc(list->next, (size_t)new_capacity * sizeof(ssize_t));
    if (new_next == NULL) {
        free(new_data);
        LOG_ERROR("ERROR DURING RECALLOC new_next");
        return ALLOCATION_ERROR;
    }

    for (ssize_t ind = list->capacity; ind < new_capacity; ind++) {
        new_next[ind]  = list->free_ind;
        list->free_ind = ind;
    }
    ssize_t* new_prev = (ssize_t*)realloc(list->prev, (size_t)new_capacity * sizeof(ssize_t));
    if (new_prev == NULL) {
        free(new_data);
        free(new_next);
        LOG_ERROR("ERROR DURING RECALLOC new_prev");
        return ALLOCATION_ERROR;
    }

    for (ssize_t ind = list->capacity; ind < new_capacity; ind++) {
        new_prev[ind] = -1;
    }

    list->data     = new_data;
    list->next     = new_next;
    list->prev     = new_prev;
    list->capacity = new_capacity;

    SAFE_CALL(list_verify(list))

    return SUCCESS;
}


error_code_e list_destroy(list_s* const list) {
    LOG("start list_destroy");

    assert(list       != NULL);
    assert(list->data != NULL);
    assert(list->next != NULL);
    assert(list->prev != NULL);

    if (is_readble_ptr(list)       != SUCCESS || is_readble_ptr(list->data) != SUCCESS ||
        is_readble_ptr(list->next) != SUCCESS || is_readble_ptr(list->prev) != SUCCESS) {
        LOG_ERROR("LIST CANT BE DESTROY, CORRUPTED POINTER list = %p list->data = %p list->next = %p", list, list->data, list->next);
        return INCORRECT_PARAM;
    }

    free(list->data);
    free(list->next);
    free(list->prev);

    list->data     = NULL;
    list->next     = NULL;
    list->prev     = NULL;
    list->elem_ind = -1;
    list->capacity = 0;

    LOG_OK("list destroy successfully");
    return SUCCESS;
}


error_code_e list_next(list_s* const list) {
    LOG("start list_next");

    assert(list       != NULL);
    assert(list->data != NULL);
    assert(list->next != NULL);
    assert(list->prev != NULL);

    SAFE_CALL(list_verify(list))

    list->elem_ind = list->next[list->elem_ind];

    LOG_OK("list next successfully elem_ind = %zd", list->elem_ind);
    return SUCCESS;
}


error_code_e list_get(const list_s* const list, list_elem_t* value) {
    LOG("start list_get");

    assert(value      != NULL);
    assert(list       != NULL);
    assert(list->data != NULL);
    assert(list->next != NULL);
    assert(list->prev != NULL);

    SAFE_CALL(list_verify(list))

    *value = list->data[list->elem_ind];

    LOG_OK("list get successfully get_value = " LST_MDF, *value);
    return SUCCESS;
}


error_code_e list_add(list_s* const list, list_elem_t value) {
    LOG("start list_add");

    assert(list       != NULL);
    assert(list->data != NULL);
    assert(list->next != NULL);
    assert(list->prev != NULL);

    SAFE_CALL(list_verify(list))

    ssize_t free_ind = list->free_ind;

    if (free_ind == -1) {
        SAFE_CALL(list_recalloc(list));
        free_ind = list->free_ind;
        assert(free_ind != -1);
    }

    list->free_ind                         = list->next[free_ind];
    list->prev[list->next[list->elem_ind]] = free_ind;
    list->data[free_ind]                   = value;
    list->next[free_ind]                   = list->next[list->elem_ind];
    list->next[list->elem_ind]             = free_ind;
    list->prev[free_ind]                   = list->elem_ind;

    LOG_OK("list add successfully add_value = " LST_MDF, value);
    return SUCCESS;
}


error_code_e list_del(list_s* const list) {
    LOG("start list_del");

    assert(list       != NULL);
    assert(list->data != NULL);
    assert(list->next != NULL);
    assert(list->prev != NULL);

    SAFE_CALL(list_verify(list))

    ssize_t del_ind = list->elem_ind;

    if (del_ind <= 0) {
        LOG_ERROR("LIST UNDERFLOW");
        return LIST_UNDERFLOW;
    }

    if (list->prev[del_ind] == -1) {
        LOG_ERROR("CANT FIND PREV_IND");
        return INCORRECT_PARAM;
    }

    list->next[list->prev[del_ind]] = list->next[del_ind];
    list->prev[list->next[del_ind]] = list->prev[del_ind];
    list->data[del_ind]             = POISON;
    list->next[del_ind]             = list->free_ind;
    list->free_ind                  = del_ind;
    list->elem_ind                  = list->prev[del_ind];
    list->prev[del_ind]             = -1;

    LOG_OK("list del successfully del_ind = %zd elem_ind = %zd", del_ind, list->elem_ind);
    return SUCCESS;
}


error_code_e list_verify(const list_s* const list) {
    list_dmp(list);

    if (list == NULL) { // TODO битовые поля
        LOG_ERROR("list = NULL");
        return NULL_PARAM;
    }
    if (is_readble_ptr(list) != SUCCESS) {
        LOG_ERROR("list ptr cant be read");
        return SEG_FAULT;
    }

    if (!isnan(list->data[0])) { // При замене POISON поменять!!!
        LOG_ERROR("list->data[0] != POISON");
        return CANARY_IS_DEAD;
    }

    if (list->elem_ind < 0 || list->elem_ind >= list->capacity) {
        LOG_ERROR("INCORRECT ELEM_IND: elem_ind = %zd", list->elem_ind);
        return INCORRECT_SIZE;
    }

    char*   visited = (char*)calloc((size_t)list->capacity, sizeof(char));
    ssize_t ind     = 0;

    while (list->next[ind]) {
        if (visited[ind]) {
            LOG_ERROR("INFINITY LOOP IN LIST");
            return LOOPING_LIST;
        }

        visited[ind] = true;
        ind          = list->next[ind];

        if (ind < 0 || ind >= list->capacity) {
            LOG_ERROR("IND OUT OD RANGE ind = %zd", ind);
            free(visited);
            return INCORRECT_SIZE;
        }
    }

    free(visited);

    for (ind = 0; ind < list->capacity; ind++) {
        SAFE_CALL(node_verify(list, ind))
    }

    return SUCCESS;
}


error_code_e node_verify(const list_s* const list, const ssize_t ind) {
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

    if (list->prev[ind] != -1) {
        SAFE_CALL(edge_verify(list, ind, list->next[ind]))
        SAFE_CALL(edge_verify(list, list->prev[ind], ind))
    }

    return SUCCESS;
}


error_code_e edge_verify(const list_s* const list, const ssize_t first_ind, const ssize_t second_ind) {
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

    if (first_ind < 0 || first_ind >= list->capacity) {
        LOG_ERROR("INCORRECT SIZE FIRST IND = %zd", first_ind);
        return INCORRECT_SIZE;
    }

    if (second_ind < 0 || second_ind >= list->capacity) {
        LOG_ERROR("INCORRECT SIZE SECOND IND = %zd", second_ind);
        return INCORRECT_SIZE;
    }

    if (list->prev[first_ind] * list->prev[second_ind] < 0) {
        LOG_ERROR("The free and occupied elements are connected");
        return INCORRECT_PARAM;
    }

    if (list->prev[first_ind] != -1 && list->next[first_ind] != second_ind) {
        LOG_ERROR("list->next[first_ind] = %zd != second_ind = %zd", list->prev[first_ind], second_ind);
        return INCORRECT_PARAM;
    }

    if (list->prev[first_ind] != -1 && list->prev[second_ind] != first_ind) {
        LOG_ERROR("list->prev[second_ind] = %zd != first_ind = %zd", list->prev[second_ind], first_ind);
        return INCORRECT_PARAM;
    }

    return SUCCESS;
}


void list_dmp(const list_s* const list) {
    LOG("list %p {", list);

    if (is_readble_ptr(list) != SUCCESS) {
        LOG_ERROR("list cant be read");
        LOG("}");
        return;
    }

    bool is_data_ok = is_readble_ptr(list->data) == SUCCESS;
    bool is_next_ok = is_readble_ptr(list->next) == SUCCESS;
    bool is_prev_ok = is_readble_ptr(list->prev) == SUCCESS;

    LOG("\tdata %p {", list->data);

    if (is_data_ok) {
        for (ssize_t ind = 0; ind < list->capacity; ind++) {
            LOG("\t\t[%zu] = " LST_MDF, ind, list->data[ind]);
        }
    } else {
        LOG_ERROR("\t\tlist->data cant be read");
    }

    LOG("\t}");
    LOG("\tnext %p {", list->next);

    if (is_next_ok) {
        for (ssize_t ind = 0; ind < list->capacity; ind++) {
            LOG("\t\t[%zu] = %zd", ind, list->next[ind]);
        }
    } else {
        LOG_ERROR("\t\tlist->next cant be read");
    }

    LOG("\t}");
    LOG("\tprev %p {", list->prev);

    if (is_prev_ok) {
        for (ssize_t ind = 0; ind < list->capacity; ind++) {
            LOG("\t\t[%zu] = %zd", ind, list->prev[ind]);
        }
    } else {
        LOG_ERROR("\t\tlist->prev cant be read");
    }

    LOG("\t}");
    LOG("\telem_ind = %zd", list->elem_ind);
    LOG("\ttail     = %zu", list->next[0]);
    LOG("\tcapacity = %zu", list->capacity);
    LOG("}");

    list_img(list);

    size_t blksize = 0;
    if (stat_get_blksize("/Users/tamerlannagaev/prodva/DOUBLY_LINKED_LIST/logs/dot/graph.svg", &blksize) != SUCCESS) {
        LOG_ERROR("STAT ERROR errno: %s", strerror(errno));
        return;
    }

    char* buffer = (char*)calloc(blksize, sizeof(char));

    int dot_file = open("./DOUBLY_LINKED_LIST/logs/dot/graph.svg", O_RDONLY);

    ssize_t count_read = 0;
    while ((count_read = read(dot_file, buffer, blksize)) > 0) {
        ssize_t new_ind = 0;

        for (ssize_t true_ind = 0; true_ind < count_read; true_ind++) {

            if (buffer[true_ind] != '\n') {
                buffer[new_ind] = buffer[true_ind];
                new_ind++;
            }
            if (true_ind >= 4 && strncmp(&buffer[true_ind - 4], "/svg>", 5) == 0) {
                break;

            }
        }

        fwrite(buffer, 1, (size_t)new_ind,  GLOBAL_LOG_FILE);
    }

    fprintf(GLOBAL_LOG_FILE, "\n");

    if (close(dot_file) == -1) {
        LOG_ERROR("CANT CLOSE dot_file");
    }

    LOG_OK("list dmp successfully");
    return;
}


void list_img(const list_s* const list) {
    assert(list       != NULL);
    assert(list->data != NULL);
    assert(list->next != NULL);
    assert(list->prev != NULL);

    bool is_data_ok = is_readble_ptr(list->data) == SUCCESS;
    bool is_next_ok = is_readble_ptr(list->next) == SUCCESS;
    bool is_prev_ok = is_readble_ptr(list->prev) == SUCCESS;

    if (!is_data_ok || !is_next_ok || !is_prev_ok) {
        LOG_ERROR("img cant be made");
        return;
    }

    FILE* dot_file = fopen("./DOUBLY_LINKED_LIST/logs/dot/graph.dot", "w");

    if (dot_file == NULL) {
        LOG_ERROR("error during open dot file");
        return;
    }

    fprintf(dot_file,   "digraph {\n"
                            "\trankdir=LR;\n"
                            "\tbgcolor=\"" BGCOLOR_HX "\";"
                            // "\tsplines=ortho\n"
                            "\tnodesep=0.6\n"
                            "\trank_sep=1.0\n"
                            );

    fprintf(dot_file, "\tnode0[shape=Mrecord; style=filled; fillcolor=\"" DARK_WINE_HX     "\"; color=\"" LAVENDER_HX      "\";fontcolor=\"" WARM_GRAY_HX "\" label=\" index = 0 (Imaginary) | data = " LST_MDF " | { next = %zd | prev = %zd }\"];\n", list->data[0], list->next[0], list->prev[0]);
    fprintf(dot_file, "\thead[shape=Mrecord;  style=filled; fillcolor=\"" HONEY_YELLOW_HX  "\"; color=\"" HONEY_YELLOW_HX  "\";fontcolor=\"" MAIN_TEXT_HX "\" label=\"head\"]\n");
    fprintf(dot_file, "\ttail[shape=Mrecord;  style=filled; fillcolor=\"" PASTEL_PURPLE_HX "\"; color=\"" PASTEL_PURPLE_HX "\";fontcolor=\"" MAIN_TEXT_HX "\" label=\"tail\"]\n");
    fprintf(dot_file, "\telem_ind[shape=Mrecord;  style=filled; fillcolor=\"" AQUA_HX      "\"; color=\"" AQUA_HX          "\";fontcolor=\"" MAIN_TEXT_HX "\" label=\"elem_ind\"]\n");


    for (ssize_t ind = 0; ind < list->capacity - 1; ind++) {
        const char* fillcolor  = NULL;
        const char* color      = NULL;

        if (node_verify(list, ind + 1) != SUCCESS) {
            fillcolor = DARK_RED_HX;
            color     = RED_HX;
        } else if (list->prev[ind + 1] == -1) {
            fillcolor = GRAPHITE_HX;
            color     = MUTED_GRAY_HX;
        } else {
            fillcolor = DARK_TEAL_HX;
            color     = AQUA_HX;
        }

        fprintf(dot_file, "\tnode%zd [shape=Mrecord; style=filled; fillcolor=\"%s\"; color=\"%s\"; fontcolor=\"%s\"  label=\" index = %zd | data = " LST_MDF " | { next = %zd | prev = %zd }\"];\n"
                          "\tnode%zd -> node%zd[style=invis; weight=1000];\n",
                          ind + 1, fillcolor, color, MAIN_TEXT_HX, ind + 1, list->data[ind + 1], list->next[ind + 1], list->prev[ind + 1],
                          ind, ind + 1);
    }

    fprintf(dot_file,     "\thead -> node%zd[color=\"" AQUA_HX "\"];\n", list->next[0]);
    fprintf(dot_file,     "\ttail -> node%zd[color=\"" AQUA_HX "\"];\n", list->prev[0]);
    fprintf(dot_file, "\telem_ind -> node%zd[color=\"" AQUA_HX "\"];\n", list->elem_ind);

    for (ssize_t ind = 0; ind < list->capacity; ind++) {
        if (list->next[ind] == -1) {
            continue;
        } else if (list->prev[ind] == -1) {
            fprintf(dot_file, "\tnode%zd -> node%zd[color=\"" MUTED_GRAY_HX "\"];\n", ind, list->next[ind]);
        } else if (edge_verify(list, ind, list->next[ind]) != SUCCESS) {
            fprintf(dot_file, "\tnode%zd -> node%zd[color=\"" RED_HX        "\"];\n", ind, list->next[ind]);
        } else {
            fprintf(dot_file, "\tnode%zd -> node%zd[color=\"" AQUA_HX       "\"];\n", ind, list->next[ind]);
        }
    }

    fprintf(dot_file, "}");

    if (fclose(dot_file) == EOF) {
        LOG_ERROR("CANT CLOSE dot_file");
    }


    int status = system("dot -Tsvg DOUBLY_LINKED_LIST/logs/dot/graph.dot -o DOUBLY_LINKED_LIST/logs/dot/graph.svg");

    if (status == -1) {
        LOG_ERROR("status of system() = -1");
        return;
    }

    LOG_OK("list img gen successfully");
    return;
}


bool is_equal(double first, double second) {
    const double EPSILON = 1e-6;
    return fabs(first - second) < EPSILON;
}
