#include "../include/DLlist.h"

int main() {
    SAFE_CALL( log_start())

               list_s list = {};
    SAFE_CALL( list_assign(&list, 3))

    SAFE_CALL( list_add(&list, 1))
    SAFE_CALL( list_add(&list, 2))
    SAFE_CALL( list_add(&list, 3))


    SAFE_CALL( list_next(&list))
    SAFE_CALL( list_next(&list))
    SAFE_CALL( list_del(&list))

    // list.next[3] = 2;

    SAFE_CALL( list_add(&list, 4))
    SAFE_CALL( list_add(&list, 5))
    SAFE_CALL( list_next(&list))
    SAFE_CALL( list_add(&list, 6))

    list_dmp(&list);

    SAFE_CALL( list_destroy(&list))
    SAFE_CALL( log_end())
    return 0;
}
