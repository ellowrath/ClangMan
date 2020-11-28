#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

static bool print_welcome_message() {
    printf("%s\n", "Welcome to ClangMan.");
    return true;
}

int main(void) {
    int status = EXIT_FAILURE;
    if (print_welcome_message()){
        status = EXIT_SUCCESS;
    }
    return status;
}