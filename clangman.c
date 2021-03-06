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
    char *lastGuess;
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
    int line_length = 0;
    struct statfs fsInfo = {0};
    int file_descriptor;
    file_descriptor = open(WORDLIST, O_RDONLY);
    long optimalSize;

    if(fstatfs(file_descriptor, &fsInfo) == -1) {
        optimalSize = 4 * 1024 * 1024;
    }
    else {
        optimalSize = fsInfo.f_bsize;
    }

    char *test_char = malloc(sizeof(*test_char) * optimalSize);
    size_t read_bytes = read(file_descriptor, test_char, optimalSize);
    unsigned int counter = 0;

    // add some error checking 

    while (read_bytes) {
        if (test_char[counter] == '\n') {
            ++line_length;
        }
        if (counter == optimalSize) {
            counter = 0;
            read_bytes = read(file_descriptor, test_char, optimalSize);
            if (read_bytes < optimalSize && read_bytes > 0) {
                optimalSize = read_bytes;
            }
        }
        ++counter;
    }

    close(file_descriptor);
    free(test_char);
    return line_length;
}

static unsigned int get_random_range(unsigned int begin, unsigned int end) {
    srand(time(0));
    return (rand() % (end - begin +1)) + begin;
}

static void set_word_by_line_num(unsigned int line) {
    int current_line = 0;
    struct statfs fsInfo = {0};
    int file_descriptor;
    file_descriptor = open(WORDLIST, O_RDONLY);
    long optimalSize;

    if(fstatfs(file_descriptor, &fsInfo) == -1) {
        optimalSize = 4 * 1024 * 1024;
    }
    else {
        optimalSize = fsInfo.f_bsize;
    }

    char *test_char = malloc(sizeof(*test_char) * optimalSize);
    char *chosen_word = malloc(100);
    size_t read_bytes = read(file_descriptor, test_char, optimalSize);
    unsigned int line_counter = 0;
    unsigned int character_counter = 0;

    while (read_bytes) {
        if (test_char[line_counter] == '\n') {
            ++current_line;
        }
        if (current_line == line) {
            chosen_word[character_counter] = test_char[line_counter+1];
            ++character_counter;
        }
        if (current_line > line && character_counter > 0) {
            chosen_word[character_counter-1] = '\0';
            game.chosenWord = chosen_word;
        }
        if (line_counter == optimalSize) {
            line_counter = 0;
            read_bytes = read(file_descriptor, test_char, optimalSize);
            if (read_bytes < optimalSize && read_bytes > 0) {
                optimalSize = read_bytes;
            }
        }
        ++line_counter;
    }
    
    char *masked_word = malloc(strlen(game.chosenWord));
    for (int i = 0; i < strlen(chosen_word); ++i) {
        masked_word[i] = '~';
    }
    masked_word[strlen(masked_word)] = '\0';
    game.maskedWord = masked_word;

    // printf("chosen word %s is %zu characters long\n", game.chosenWord, strlen(game.chosenWord));
    // printf("masked word %s is %zu characters long\n", game.maskedWord, strlen(game.maskedWord));

    close(file_descriptor);
    free(test_char);
}

static void prepare_game_state() {
    game.success = false;
    game.missed = 0;
    game.guesses = 0;
    game.guessedChars = calloc(sizeof(char), strlen(game.chosenWord) + MAXMISSEDGUESSES);
    game.lastGuess = calloc(sizeof(char), 1);
}

static void get_guess() {
    printf("Please enter a single character guess: ");
    fflush(stdout);
    fgets(game.lastGuess, 2, stdin);
    while(fgetc(stdin) != '\n');
}

static void process_successful_guess(char c) {
    for (int i = 0; i < strlen(game.chosenWord); ++i) {
        if (c == game.chosenWord[i]) {
            game.maskedWord[i] = game.chosenWord[i];
        }
    }
}

static void game_loop() {
    // debug_game_state();
    get_guess();
    if (strstr(game.guessedChars, game.lastGuess) != NULL) {
        printf("You've already guessed that letter, please try again.\n");
    }
    else {
        if(strstr(game.chosenWord, game.lastGuess) != NULL) {
            process_successful_guess(*game.lastGuess);
            printf("Good stuff! %c is a good guess.\n", *game.lastGuess);
            if (strcmp(game.chosenWord, game.maskedWord) ==0) {
                game.running = false;
            }
        }
        else {
            printf("I'm sorry! %c is not a good guess.\n", *game.lastGuess);
            ++game.missed;
        }
        game.guessedChars[strlen(game.guessedChars)] = *game.lastGuess;
        ++game.guesses;
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
        --name_attempts;
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
        if (strcmp(game.chosenWord, game.maskedWord) == 0) {
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
