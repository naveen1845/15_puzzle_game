#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include "frontend.h"

static struct termios orig_termios;
static bool raw_mode_enabled = false;

/*
 * FRONT-END: Terminal Raw Mode
 * Allows capturing single keystrokes (like arrow keys, W/A/S/D)
 * immediately without waiting for the user to press 'Enter'.
 */
void enableRawMode(void) {
    if (raw_mode_enabled) return;
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON); /* Disable echo and canonical mode */
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    raw_mode_enabled = true;
}

void disableRawMode(void) {
    if (!raw_mode_enabled) return;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    raw_mode_enabled = false;
}

/*
 * Reads a single keypress, seamlessly translating ANSI escape sequences
 * for Arrow keys into standard 'W', 'A', 'S', 'D' directions.
 */
int getKeyPress(void) {
    char c = 0;
    if (read(STDIN_FILENO, &c, 1) <= 0) return 0;

    if (c == '\033') { /* Escape sequence (e.g. arrow keys) */
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) <= 0) return '\033';
        if (read(STDIN_FILENO, &seq[1], 1) <= 0) return '\033';

        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return 'W'; /* Up arrow */
                case 'B': return 'S'; /* Down arrow */
                case 'C': return 'D'; /* Right arrow */
                case 'D': return 'A'; /* Left arrow */
            }
        }
        return '\033';
    }
    return c;
}

void clearScreen(void) {
    printf("\033[2J\033[H");
}

void renderBanner(void) {
    printf("%s%s", ANSI_CYAN, ANSI_BOLD);
    printf("=============================================\n");
    printf("   ___  __  __________   __    ____     \n");
    printf("  / _ \\/ / / /_  /_  /  / /   / __/     \n");
    printf(" / ___/ /_/ / / / / /__/ /___/ _/       \n");
    printf("/_/   \\____/ /___/____/_____/___/       \n");
    printf("=============================================\n");
    printf("%s", ANSI_RESET);
}

void renderMenu(bool canResume) {
    clearScreen();
    renderBanner();
    printf("\n%s%s           --- MAIN MENU ---%s\n\n", ANSI_BOLD, ANSI_YELLOW, ANSI_RESET);
    printf("  %s[1]%s Start New Game\n", ANSI_BOLD, ANSI_RESET);
    if (canResume) {
        printf("  %s[2]%s Resume Saved Game %s(Saved File Found)%s\n", ANSI_BOLD, ANSI_RESET, ANSI_GREEN, ANSI_RESET);
    } else {
        printf("  %s[2]%s %sResume Saved Game (None available)%s\n", ANSI_BOLD, ANSI_RESET, ANSI_DIM, ANSI_RESET);
    }
    printf("  %s[3]%s View Hall of Fame (Leaderboard)\n", ANSI_BOLD, ANSI_RESET);
    printf("  %s[4]%s How to Play / Rules\n", ANSI_BOLD, ANSI_RESET);
    printf("  %s[5]%s Exit\n\n", ANSI_BOLD, ANSI_RESET);
    printf("%sSelect an option [1-5]: %s", ANSI_CYAN, ANSI_RESET);
    fflush(stdout);
}

/*
 * SYLLABUS TOPIC: PATTERN PROGRAMMING & 2D ARRAYS
 * Renders an aesthetically styled box grid for the 4x4 tiles.
 * Correctly placed tiles are highlighted in green to guide the user.
 */
void renderBoard(const struct GameState *game) {
    clearScreen();
    printf("%s%s", ANSI_CYAN, ANSI_BOLD);
    printf("=====================================================\n");
    printf("                THE 15-PUZZLE CHALLENGE              \n");
    printf("=====================================================\n%s", ANSI_RESET);

    /* HUD / Status Bar */
    int minutes = game->elapsedSeconds / 60;
    int seconds = game->elapsedSeconds % 60;

    printf("%sPlayer: %s%-14s%s | %sMoves: %s%-5d%s | %sTime: %s%02d:%02d%s\n",
           ANSI_BOLD, ANSI_YELLOW, game->playerName, ANSI_RESET,
           ANSI_BOLD, ANSI_MAGENTA, game->moves, ANSI_RESET,
           ANSI_BOLD, ANSI_CYAN, minutes, seconds, ANSI_RESET);
    printf("-----------------------------------------------------\n\n");

    /* Box Drawing Grid */
    printf("       ┌──────┬──────┬──────┬──────┐\n");

    for (int r = 0; r < SIZE; r++) {
        printf("       │");
        for (int c = 0; c < SIZE; c++) {
            int val = game->board[r][c];
            if (val == 0) {
                /* Blank space */
                printf("%s  ░░  %s│", ANSI_DIM, ANSI_RESET);
            } else {
                /* Check if tile is in its target solved position */
                int targetVal = r * SIZE + c + 1;
                if (val == targetVal) {
                    /* Highlight correct placement in Green */
                    printf("  %s%s%2d%s  │", ANSI_BOLD, ANSI_GREEN, val, ANSI_RESET);
                } else {
                    /* Unsolved tile in Yellow */
                    printf("  %s%s%2d%s  │", ANSI_BOLD, ANSI_YELLOW, val, ANSI_RESET);
                }
            }
        }
        printf("\n");

        if (r < SIZE - 1) {
            printf("       ├──────┼──────┼──────┼──────┤\n");
        }
    }
    printf("       └──────┴──────┴──────┴──────┘\n\n");

    /* Controls Legend */
    printf("%sControls:%s [W/▲ Up] [S/▼ Down] [A/◄ Left] [D/► Right]\n", ANSI_BOLD, ANSI_RESET);
    printf("          [P] Save Game   [Q] Quit to Menu\n");
}

void renderWinScreen(const struct GameState *game) {
    clearScreen();
    printf("\n\n%s%s", ANSI_GREEN, ANSI_BOLD);
    printf("  ***************************************************\n");
    printf("  *                                                 *\n");
    printf("  *       CONGRATULATIONS! PUZZLE SOLVED!           *\n");
    printf("  *                                                 *\n");
    printf("  ***************************************************\n%s", ANSI_RESET);

    int minutes = game->elapsedSeconds / 60;
    int seconds = game->elapsedSeconds % 60;

    printf("\n  Player Name : %s%s%s\n", ANSI_YELLOW, game->playerName, ANSI_RESET);
    printf("  Total Moves : %s%d moves%s\n", ANSI_MAGENTA, game->moves, ANSI_RESET);
    printf("  Total Time  : %s%02d:%02d%s\n\n", ANSI_CYAN, minutes, seconds, ANSI_RESET);
    printf("  Your score has been saved to the Hall of Fame!\n\n");
    printf("  Press any key to return to Main Menu... ");
    fflush(stdout);
}

void renderLeaderboard(const struct LeaderboardEntry entries[], int count) {
    clearScreen();
    renderBanner();
    printf("\n%s%s               --- HALL OF FAME ---%s\n", ANSI_BOLD, ANSI_YELLOW, ANSI_RESET);
    printf("%s(Ranked by: Fewest Moves, then Lowest Time using Selection Sort)%s\n\n", ANSI_DIM, ANSI_RESET);

    if (count == 0) {
        printf("   No high scores recorded yet! Solve a puzzle to rank here.\n\n");
    } else {
        printf("  ┌──────┬──────────────────────┬─────────┬──────────┬────────────┐\n");
        printf("  │ %-4s │ %-20s │ %-7s │ %-8s │ %-10s │\n",
               "Rank", "Player Name", "Moves", "Time", "Date");
        printf("  ├──────┼──────────────────────┼─────────┼──────────┼────────────┤\n");

        for (int i = 0; i < count; i++) {
            int m = entries[i].timeSpentSeconds / 60;
            int s = entries[i].timeSpentSeconds % 60;
            char timeBuf[16];
            snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", m, s);

            const char *rankColor = (i == 0) ? ANSI_YELLOW : (i == 1 ? ANSI_WHITE : ANSI_RESET);

            printf("  │ %s%-4d%s │ %-20s │ %-7d │ %-8s │ %-10s │\n",
                   rankColor, i + 1, ANSI_RESET,
                   entries[i].name,
                   entries[i].moves,
                   timeBuf,
                   entries[i].date);
        }
        printf("  └──────┴──────────────────────┴─────────┴──────────┴────────────┘\n\n");
    }

    printf("Press any key to return to Main Menu... ");
    fflush(stdout);
}

void renderHelp(void) {
    clearScreen();
    renderBanner();
    printf("\n%s%s           --- HOW TO PLAY THE 15-PUZZLE ---%s\n\n", ANSI_BOLD, ANSI_YELLOW, ANSI_RESET);
    printf("  1. The board is a 4x4 grid containing tiles numbered 1 to 15\n");
    printf("     and one empty space (marked as ░░).\n\n");
    printf("  2. Objective:\n");
    printf("     Arrange all tiles in ascending order from 1 to 15,\n");
    printf("     leaving the blank space in the bottom-right corner:\n\n");
    printf("       [ 1] [ 2] [ 3] [ 4]\n");
    printf("       [ 5] [ 6] [ 7] [ 8]\n");
    printf("       [ 9] [10] [11] [12]\n");
    printf("       [13] [14] [15] [░░]\n\n");
    printf("  3. Tiles highlighted in %sGREEN%s are already in their correct spot!\n", ANSI_GREEN, ANSI_RESET);
    printf("  4. Use %sW, A, S, D%s or %sArrow Keys%s to slide adjacent tiles into the blank space.\n",
           ANSI_BOLD, ANSI_RESET, ANSI_BOLD, ANSI_RESET);
    printf("  5. Press %sP%s at any time during gameplay to save and resume later.\n\n", ANSI_BOLD, ANSI_RESET);
    printf("Press any key to return to Main Menu... ");
    fflush(stdout);
}

void renderMessage(const char *msg, const char *color) {
    printf("\n%s%s%s\n", color, msg, ANSI_RESET);
    fflush(stdout);
}
