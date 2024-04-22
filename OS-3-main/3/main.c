#include <stdio.h>
#include <unistd.h>
#include "../2/proactor.h"

int main() {
    proactor_t *proactor = proactor_create(9034);

    // Run server indefinitely
    while (1) {
        sleep(1);
    }

    proactor_destroy(proactor);
    return 0;
}