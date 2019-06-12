#include <stdio.h>
#include "zenoh/collection.h"

int main(int argc, char** arg) {

    z_list_t *xs = z_list_of("one");
    xs = z_list_cons(xs, "two");
    xs = z_list_cons(xs, "three");
    printf("list len = %u\n", z_list_len(xs));
    xs = z_list_drop_elem(xs, 1);
    printf("list len = %u\n", z_list_len(xs));
    z_list_free(&xs);
    return 0;
}