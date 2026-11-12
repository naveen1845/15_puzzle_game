#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "backend.h"
#include "frontend.h"

/* Helper to get current date formatted as YYYY-MM-DD */
static void getCurrentDate(char *buf, size_t bufSize) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buf, bufSize, "%Y-%m-%d", t);
}

/*
 * Game Loop for an active puzzle session
 */
static void runGameSession(struct GameState *game) {
    enableRawMode();

    while (!game->isWon) {
        renderBoard(game);

        int key = getKeyPress();

        if (key == 'q' || key == 'Q') {
            /* Ask user to confirm quit / save */
            disableRawMode();
            printf("\nDo you want to save your progress before quitting? (y/n): ");
            char choice = getchar();
            while (getchar() != '\n'); /* Flush newline */

            if (choice == 'y' || choice == 'Y') {
                saveGameProgress(game);
                printf("%sGame progress saved!%s\n", ANSI_GREEN, ANSI_RESET);
            }
            return;
        } else if (key == 'p' || key == 'P') {
            saveGameProgress(game);
            renderBoard(game);
            printf("\n%s>> Game successfully saved! Press any move key to continue playing... <<%s",
                   ANSI_GREEN, ANSI_RESET);
            fflush(stdout);
            getKeyPress();
        } else if (key == 'w' || key == 'W' ||
                   key == 's' || key == 'S' ||
                   key == 'a' || key == 'A' ||
                   key == 'd' || key == 'D') {
            moveTile(game, (char)key);
        }
    }

    /* If won */
    disableRawMode();

    /* Record score in Hall of Fame */
    struct LeaderboardEntry entry;
    strncpy(entry.name, game->playerName, MAX_NAME_LEN - 1);
    entry.name[MAX_NAME_LEN - 1] = '\0';
    entry.moves = game->moves;
    entry.timeSpentSeconds = game->elapsedSeconds;
    getCurrentDate(entry.date, sizeof(entry.date));

    saveScore(&entry);

    /* Delete saved game file if game was finished */
    remove(SAVE_FILE);

    renderWinScreen(game);
    enableRawMode();
    getKeyPress();
    disableRawMode();
}

int main(void) {
    /* Ensure raw mode is cleanly disabled when program exits */
    atexit(disableRawMode);

    while (1) {
        bool canResume = hasSavedGame();
        renderMenu(canResume);

        char input[10];
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }

        int choice = atoi(input);

        if (choice == 1) {
            /* Start New Game */
            char name[MAX_NAME_LEN];
            printf("\nEnter your player name: ");
            if (fgets(name, sizeof(name), stdin)) {
                name[strcspn(name, "\r\n")] = '\0';
            }
            if (strlen(name) == 0) {
                strcpy(name, "Player1");
            }

            struct GameState game;
            initBoard(&game, name);
            shuffleBoard(&game);

            runGameSession(&game);

        } else if (choice == 2) {
            /* Resume Game */
            if (!canResume) {
                printf("\n%sNo saved game found!%s\n", ANSI_RED, ANSI_RESET);
                printf("Press Enter to continue...");
                getchar();
                continue;
            }

            struct GameState game;
            if (loadGameProgress(&game)) {
                printf("\n%sResuming saved game for %s...%s\n", ANSI_GREEN, game.playerName, ANSI_RESET);
                runGameSession(&game);
            } else {
                printf("\n%sFailed to load saved game!%s\n", ANSI_RED, ANSI_RESET);
                getchar();
            }

        } else if (choice == 3) {
            /* View Leaderboard */
            struct LeaderboardEntry entries[MAX_LEADERBOARD_ENTRIES];
            int count = loadLeaderboard(entries, MAX_LEADERBOARD_ENTRIES);

            enableRawMode();
            renderLeaderboard(entries, count);
            getKeyPress();
            disableRawMode();

        } else if (choice == 4) {
            /* How to Play */
            enableRawMode();
            renderHelp();
            getKeyPress();
            disableRawMode();

        } else if (choice == 5) {
            /* Exit */
            clearScreen();
            printf("\n%sThanks for playing The 15-Puzzle Challenge! Goodbye!%s\n\n",
                   ANSI_CYAN, ANSI_RESET);
            break;
        } else {
            printf("\n%sInvalid option! Please enter 1 to 5.%s\n", ANSI_RED, ANSI_RESET);
            printf("Press Enter to continue...");
            getchar();
        }
    }

    return 0;
}
