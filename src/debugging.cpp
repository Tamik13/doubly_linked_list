#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>

#include "../include/debugging.h"

const char* const LOG_FILE_NAME   = "./DOUBLY_LINKED_LIST/logs/log.txt";
FILE*             GLOBAL_LOG_FILE = NULL;


error_code_e log_start() {
    GLOBAL_LOG_FILE = fopen(LOG_FILE_NAME, "w");

    if (GLOBAL_LOG_FILE == NULL) {
        PRINT_ERROR(ERROR_DURING_OPEN);
        return ERROR_DURING_OPEN;
    }

    return SUCCESS;
}

void _log(const char*  prefix, const char* file,    const char* function,
                size_t line,   const char* message, ...) {
    assert(message != NULL);
    const size_t MAX_TIME_SIZE = 100;

    if (GLOBAL_LOG_FILE == NULL) {
        return;
    }

    va_list ap = {};
    va_start(ap, message);

    time_t now    = time(NULL);
    struct tm *tp = localtime(&now);

    char time_string[MAX_TIME_SIZE] = {};
    strftime(time_string, MAX_TIME_SIZE, "%H:%M:%S", tp);

    fprintf (GLOBAL_LOG_FILE, "[%-10s] [%-4zu] [%-27s] [%s] [%5s] ->  ", file, line, function, time_string, prefix);
    vfprintf(GLOBAL_LOG_FILE, message, ap);
    va_end  (ap);
    fprintf (GLOBAL_LOG_FILE, "\n");
}

error_code_e log_end() {
    if (fclose(GLOBAL_LOG_FILE) == EOF) {
        PRINT_ERROR(ERROR_DURING_CLOSE);
        return ERROR_DURING_CLOSE;
    }

    return SUCCESS;
}


unsigned long djb2_hash(const unsigned char* str, const size_t size) {
    assert(str != NULL);

    unsigned long hash = 5381;

    for (size_t ind = 0; ind < size; ind++) {
        hash = ((hash << 5) + hash) + str[ind];
    }

    return hash;
}

unsigned long djb2_str_hash(const unsigned char* str) {
    assert(str != NULL);

    unsigned long hash = 5381;

    while (*str) {
        hash = ((hash << 5) + hash) + *str++;
    }

    return hash;
}


const char* my_str_error(const error_code_e error_code) {
    switch (error_code) {
        case SUCCESS:
            return "SUCCESS: 0";

        case INCORRECT_SIZE:
            return "INCORRECT SIZE: 1";

        case NULL_PARAM:
            return "NULL PARAM: 2";

        case ALLOCATION_ERROR:
            return "ALLOCATION ERROR: 3";

        case LIST_OVERFLOW:
            return "LIST_OVERFLOW: 4";

        case ERROR_DURING_OPEN:
            return "ERROR DURING OPEN: 5";

        case ERROR_DURING_CLOSE:
            return "ERROR DURING CLOSE: 6";

        case CANARY_IS_DEAD:
            return "CANARY IS DEAD: 7";

        case REINITIALIZATION:
            return "REINITIALIZATION: 8";

        case HASH_CHANGED:
            return "HASH CHANGED: 9";

        case SEG_FAULT:
            return "SEG FAULT: 10";

        case ERROR_DURING_READ:
            return "ERROR DURING READ: 11";

        case DIVISION_BY_ZERO:
            return "DIVISION BY ZERO: 12";

        case STAT_READ_ERROR:
            return "STAT READ ERROR: 13";

        case END_OF_FILE:
            return "END_OF_FILE: 14";

        case INCORRECT_PARAM:
            return "INCORRECT_PARAM: 15";

        case DONT_OPEN_LOG:
            return "DONT OPEN LOG: 16";

        case LIST_UNDERFLOW:
            return "LIST_UNDERFLOW: 17";

        case INIT_VALUE:
            return "INIT VALUE: -1";

        case UNEXPECTED_ERROR:
        default:
            return "UNEXPECTED ERROR: -2";
    }
}


error_code_e is_readble_ptr(const void* ptr) {
    int file_descriptor[2] = {0, 0};

    if (pipe(file_descriptor) == -1) {
        LOG_ERROR("cant open file descriptor");
        return ERROR_DURING_OPEN;
    }

    ssize_t is_file_open = 0;

    do {
        is_file_open = write(file_descriptor[1], ptr, 1);
    } while (is_file_open == -1 && errno == EINTR);

    int saved_errno = errno;

    if (close(file_descriptor[0])) {
        LOG_ERROR("FILE_DESCRIPTOR[0] CANT BE CLOSE");
    }

    if (close(file_descriptor[1])) {
        LOG_ERROR("FILE_DESCRIPTOR[1] CANT BE CLOSE");
    }

    if (is_file_open == 1) {
        return SUCCESS;
    }

    errno = saved_errno;

    if (is_file_open == -1 && saved_errno == EFAULT) {

        LOG_ERROR("pointer cant be read");

        return SEG_FAULT;
    }

    LOG_ERROR("unexpected error");
    return UNEXPECTED_ERROR;
}


error_code_e stat_get_size(const char* const file_name, size_t* const file_size) {
    struct stat buff = {};

    if (stat(file_name, &buff) == -1) {
        LOG_ERROR("STAT OPEN ERROR");
        return STAT_READ_ERROR;
    }

    *file_size = (size_t)buff.st_size;

    return SUCCESS;
}

void _$(const int           num) {fprintf(stderr, "%d\n\n",       num);}
void _$(const double        num) {fprintf(stderr, "%lg\n\n",      num);}
void _$(const unsigned long num) {fprintf(stderr, "%lu\n\n",      num);}
void _$(const long long     num) {fprintf(stderr, "%lld\n\n",     num);}
void _$(const void* const   ptr) {fprintf(stderr, "%p\n\n",       ptr);}
void _$(const char* const   str) {fprintf(stderr, "<%s>\n\n",     str);}
void _$(const char       symbol) {fprintf(stderr, "<%c>, %d\n\n", symbol, symbol);}
void _$(const long          num) {fprintf(stderr, "%ld\n\n",      num);}
