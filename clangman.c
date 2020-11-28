#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define NAMESIZE 13

typedef struct {
    char playerName[NAMESIZE];
} gameState;

gameState game;

static bool print_welcome_message() {
    printf("%s\n", "Welcome to ClangMan.");
    return true;
}

static bool get_player_name() {
    printf("%s\n", "Please enter your name, 12 characters or less: ");
    fgets(game.playerName, NAMESIZE, stdin);
    if (strlen(game.playerName) < 2) {
        return false;
    }
    return true;
}

static bool print_name_error() {
    printf("%s\n", "You've failed to enter your name 3 times. Game terminated!");
    return true;
}

static bool print_player_name() {
    printf("%s\n", game.playerName);
    return true;
}

int main(void) {
    int status = EXIT_FAILURE;
    int name_attempts = 3;

    if (print_welcome_message()){
        status = EXIT_SUCCESS;
    }

    while (name_attempts > 0 && !get_player_name()) {
        name_attempts--;
    }
    
    if (name_attempts == 0) {
        status = print_name_error();
    }
    else {
        status = print_player_name();
    }
    return status;
}