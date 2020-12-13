#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/vfs.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#define NAMESIZE 13
#define WORDLIST "wordlist.txt"
#define NAMEATTEMPTS 3

typedef struct {
    char playerName[NAMESIZE];
    char *chosenWord;
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

static bool print_chosen_word() {
    printf("%s\n", game.chosenWord);
    return true;
}

static unsigned int get_file_line_length() {
    int ret = 0;
    struct statfs fsInfo = {0};
    int fd;
    fd = open(WORDLIST, O_RDONLY);
    // DEBUG
    printf("FILE DESCRIPTOR (get_file_line_length): %d\n", fd);
    long optimalSize;

    if(fstatfs(fd, &fsInfo) == -1) {
        optimalSize = 4 * 1024 * 1024;
    }
    else {
        optimalSize = fsInfo.f_bsize;
    }
    // DEBUG
    printf("OPTIMAL SIZE: %ld\n", optimalSize);

    char *p = malloc(sizeof(*p) * optimalSize);
    size_t read_bytes = read(fd, p, optimalSize);
    unsigned int i = 0;

    // add some error checking 

    while (read_bytes) {
        if (p[i] == '\n') {
            ret++;
        }
        if (i == optimalSize) {
            i = 0;
            read_bytes = read(fd, p, optimalSize);
            if (read_bytes < optimalSize && read_bytes > 0) {
                optimalSize = read_bytes;
            }
        }
        i++;
    }

    close(fd);
    free(p);
    return ret;
}

static unsigned int get_random_range(unsigned int begin, unsigned int end) {
    srand(time(0));
    return (rand() % (end - begin +1)) + begin;
}

static bool set_word_by_line_num(unsigned int line) {
    bool ret = false;
    int current_line = 0;
    struct statfs fsInfo = {0};
    int fd;
    fd = open(WORDLIST, O_RDONLY);
    // DEBUG
    printf("FILE DESCRIPTOR (get_word_by_line_num): %d\n", fd);
    long optimalSize;

    if(fstatfs(fd, &fsInfo) == -1) {
        optimalSize = 4 * 1024 * 1024;
    }
    else {
        optimalSize = fsInfo.f_bsize;
    }

    char *p = malloc(sizeof(*p) * optimalSize);
    char *chosen_word = malloc(100);
    size_t read_bytes = read(fd, p, optimalSize);
    unsigned int i = 0;
    unsigned int j = 0;

    while (read_bytes) {
        if (p[i] == '\n') {
            current_line++;
        }
        if (current_line == line) {
            chosen_word[j] = p[i];
            j++;
        }
        if (current_line > line && j > 0) {
            chosen_word[j] = '\0';
            game.chosenWord = chosen_word;
            ret = true;
        }
        if (i == optimalSize) {
            i = 0;
            read_bytes = read(fd, p, optimalSize);
            if (read_bytes < optimalSize && read_bytes > 0) {
                optimalSize = read_bytes;
            }
        }
        i++;
    }

    close(fd);
    free(p);
    return ret;
}

int main(void) {
    int status = EXIT_FAILURE;
    int name_attempts = NAMEATTEMPTS;
    unsigned int line_length;
    unsigned int chosen_line;

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
    line_length = get_file_line_length();
    chosen_line = get_random_range(0, line_length);
    if (set_word_by_line_num(chosen_line)) {
        status = print_chosen_word();
    }

    return status;
}