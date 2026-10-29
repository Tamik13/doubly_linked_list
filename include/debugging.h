#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdio.h>

#define PRINT_ERROR(error)                                                                                      \
    fprintf(stderr, "%s:%d " COLOR_TEXT("ERROR CODE: ", RED_D) "%s   " COLOR_TEXT("ERRNO: ", RED_D) "%s  %s\n", \
     __FILE__, __LINE__, my_str_error(error), strerror(errno), __FUNCTION__)

#define ASSERT_FOR_ARR(ind, size) assert(0 <= (size_t)ind && (size_t)ind < (size_t)size)

#define SAFE_CALL(fun_call) {                                                           \
        error_code_e error_code = fun_call;                                             \
        if (error_code != SUCCESS) {                                                    \
            LOG_ERROR("ERROR IN SAFE CALL %s: %s", #fun_call, my_str_error(error_code));\
            return error_code;                                                          \
        }                                                                               \
    }

#define $ANCHOR         fprintf(stderr, "%s:%d " COLOR_TEXT("ANCHOR",         VIOLET_D) " %s\n", __FILE__, __LINE__, __FUNCTION__)
#define $START_FUNCTION fprintf(stderr, "%s:%d " COLOR_TEXT("START FUNCTION", BLUE_D)   " %s\n", __FILE__, __LINE__, __FUNCTION__)
#define $END_FUNCTION   fprintf(stderr, "%s:%d " COLOR_TEXT("END FUNCTION",   YELLOW_D) " %s\n", __FILE__, __LINE__, __FUNCTION__)

#define RED_D    "91"
#define GREEN_D  "92"
#define BLUE_D   "94"
#define YELLOW_D "33"
#define VIOLET_D "35"

#define COLOR_TEXT(STR, COLOR)  "\033[" COLOR "m" STR "\033[0m"

enum error_code_e {
    SUCCESS              = 0,
    INCORRECT_SIZE       = 1,
    NULL_PARAM           = 2,
    ALLOCATION_ERROR     = 3,
    LIST_OVERFLOW        = 4,
    ERROR_DURING_OPEN    = 5,
    ERROR_DURING_CLOSE   = 6,
    CANARY_IS_DEAD       = 7,
    REINITIALIZATION     = 8,
    HASH_CHANGED         = 9,
    SEG_FAULT            = 10,
    ERROR_DURING_READ    = 11,
    DIVISION_BY_ZERO     = 12,
    STAT_READ_ERROR      = 13,
    END_OF_FILE          = 14,
    INCORRECT_PARAM      = 15,
    DONT_OPEN_LOG        = 16,
    LIST_UNDERFLOW       = 17,
    INIT_VALUE           = -1,
    UNEXPECTED_ERROR     = -2,
};

#define LOG_ERROR(message, ...) _log("ERROR", __FILE__, __func__, __LINE__, message __VA_OPT__(,) __VA_ARGS__)
#define    LOG_OK(message, ...) _log("OK",    __FILE__, __func__, __LINE__, message __VA_OPT__(,) __VA_ARGS__)
#define       LOG(message, ...) _log("INFO",  __FILE__, __func__, __LINE__, message __VA_OPT__(,) __VA_ARGS__)

error_code_e log_start();
void _log(const char* prefix, const char* file,   const char* function,
                size_t   line,   const char* message, ...);
error_code_e log_end  ();

unsigned long djb2_hash    (const unsigned char* str, const size_t size);
unsigned long djb2_str_hash(const unsigned char* str);

const char* my_str_error(const error_code_e error_code);

error_code_e is_readble_ptr(const void*        ptr);
error_code_e stat_get_size (const char*  const file_name, size_t* const file_size);

#define $(param) {$ANCHOR; fprintf(stderr, COLOR_TEXT(#param, VIOLET_D) " = "); _$(param);}
void _$(const int           num);
void _$(const double        num);
void _$(const unsigned long num);
void _$(const long long     num);
void _$(const void* const   ptr);
void _$(const char* const   str);
void _$(const char          symbol);
void _$(const long          num);

#endif
