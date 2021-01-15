#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/vfs.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

// static const unsigned int NAMESIZE = 13;
// even static const, the above gives me variably modified at file scope error
enum {NAMESIZE = 13};

static const unsigned int MAXMISSEDGUESSES = 6;
static const char WORDLIST[] = "wordlist.txt";
static const char MASKEDWORD[] = "Guess this word:"; // used in two places, rethink that.

typedef struct {
    char playerName[NAMESIZE];
    char *chosenWord;
    char *maskedWord;
    char *guessedChars;
    bool running;
    bool success;
    unsigned int guesses;
    unsigned int missed;
} gameState;

static gameState game;

static void debug_game_state() {
    printf("DEBUGGING THE GAME STATE\n");
    printf("Player Name: %s\n", game.playerName);
    printf("The chosen word: %s\n", game.chosenWord);
    printf("The masked word: %s\n", game.maskedWord);
    printf("The guess chars: %s\n", game.guessedChars);
    printf("Length of guess chars: %ld\n", strlen(game.guessedChars));
    printf("The number of guesses: %d\n", game.guesses);
    printf("The number of misses: %d\n", game.missed);
    printf("END OF DEBUGGING\n\n");
    fflush(stdout);
}

static void flush_stdin() {
    while(fgetc(stdin) != '\n');
}

static bool get_player_name() {
    printf("Please enter your name, 12 characters or less: \n");
    fgets(game.playerName, NAMESIZE, stdin);
    if (strlen(game.playerName) < 2) {
        return false;
    }
    game.playerName[strcspn(game.playerName, "\n")] = 0;
    return true;
}

static unsigned int get_file_line_length() {
    int ret = 0;
    struct statfs fsInfo = {0};
    int fd;
    fd = open(WORDLIST, O_RDONLY);
    long optimalSize;

    if(fstatfs(fd, &fsInfo) == -1) {
        optimalSize = 4 * 1024 * 1024;
    }
    else {
        optimalSize = fsInfo.f_bsize;
    }

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
            chosen_word[j] = p[i+1];
            j++;
        }
        if (current_line > line && j > 0) {
            chosen_word[j-1] = '\0';
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
    
    char *masked_word = malloc(strlen(game.chosenWord));

    for (i = 0; i < strlen(chosen_word); i++) {
        masked_word[i] = '~';
    }
    masked_word[i] = '\0';
    game.maskedWord = masked_word;

    // printf("chosen word %s is %zu characters long\n", game.chosenWord, strlen(game.chosenWord));
    // printf("masked word %s is %zu characters long\n", game.maskedWord, strlen(game.maskedWord));

    close(fd);
    free(p);
    return ret;
}

static bool prepare_game_state() {
    game.success = false;
    game.missed = 0;
    game.guesses = 0;
    game.guessedChars = calloc(sizeof(char), strlen(game.chosenWord) + MAXMISSEDGUESSES);
    return true;
}

static char get_guess() {
    char c;
    printf("Please enter a single character guess: ");
    fflush(stdout);
    c = fgetc(stdin);
    flush_stdin();
    return c;
}

static bool is_char_in_string(char c, char* char_array) {
    bool ret = false;
    for (int i = 0; i < strlen(char_array); i++) {
        if (c == char_array[i]) {
            ret = true;
        }
    }
    return ret;
}

static void process_successful_guess(char c) {
    for (int i = 0; i < strlen(game.chosenWord); i++) {
        if (c == game.chosenWord[i]) {
            game.maskedWord[i] = game.chosenWord[i];
        }
    }
}

static bool evaluate_victory() {
    if (strcmp(game.chosenWord, game.maskedWord) == 0) {
        return true;
    }
    else {
        return false;
    }
}

static void game_loop() {
    // debug_game_state();
    char guess = get_guess();
    if (is_char_in_string(guess, game.guessedChars)) {
        printf("You've already guessed that letter, please try again.\n");
    }
    else {
        if(is_char_in_string(guess, game.chosenWord)) {
            process_successful_guess(guess);
            printf("Good stuff! %c is a good guess.\n", guess);
            if (evaluate_victory()) {
                game.running = false;
            }
        }
        else {
            printf("I'm sorry! %c is not a good guess.\n", guess);
            game.missed++;
        }
        game.guessedChars[strlen(game.guessedChars)] = guess;
        game.guesses++;
        printf("%s %s\n", MASKEDWORD, game.maskedWord);
        printf("These are you past guesses: %s\n", game.guessedChars);
    }

    if (game.missed >= MAXMISSEDGUESSES) {
        game.running = false;
        // debug_game_state();
    }

}

int main(void) {
    int status = EXIT_FAILURE;
    int name_attempts = 3;
    unsigned int chosen_line;
    game.running = true;

    printf("Welcome to ClangMan\n");

    while (name_attempts > 0 && !get_player_name()) {
        name_attempts--;
    }
    
    if (name_attempts == 0) {
        printf("You've failed to enter your name 3 times, Game terminated!\n");
        status = EXIT_SUCCESS;
    }
    else {
        printf("%s\n", game.playerName);
    }

    if (status) {
        chosen_line = get_random_range(0, get_file_line_length());
        set_word_by_line_num(chosen_line);

        prepare_game_state();
        printf("%s %s\n", MASKEDWORD, game.maskedWord);

        while (game.running) {
            game_loop();
        }
        if (evaluate_victory()) {
            printf("Congratulations %s, you guessed \"%s\" in %d.\n", game.playerName, game.chosenWord, game.guesses);
            printf("You had %d guesses left before failure. Good job!\n", MAXMISSEDGUESSES - game.missed);
        }
        else {
            printf("I'm sorry to report that you have failed to guess \"%s\".\n", game.chosenWord); 
            printf("Good luck next time %s.\n", game.playerName);
        }

    }

    return status;
}
