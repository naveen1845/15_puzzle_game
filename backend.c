#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "backend.h"

/* 
 * SYLLABUS TOPIC: 2D ARRAYS & STRUCTURES
 * Initializes the board to solved state:
 *  1   2   3   4
 *  5   6   7   8
 *  9  10  11  12
 * 13  14  15   0 (0 represents the blank space)
 */
void initBoard(struct GameState *game, const char *playerName) {
    int counter = 1;
    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
            if (r == SIZE - 1 && c == SIZE - 1) {
                game->board[r][c] = 0; /* Blank tile */
            } else {
                game->board[r][c] = counter++;
            }
        }
    }
    game->emptyRow = SIZE - 1;
    game->emptyCol = SIZE - 1;
    game->moves = 0;
    game->startTime = time(NULL);
    game->elapsedSeconds = 0;
    game->isWon = false;

    if (playerName && strlen(playerName) > 0) {
        strncpy(game->playerName, playerName, MAX_NAME_LEN - 1);
        game->playerName[MAX_NAME_LEN - 1] = '\0';
    } else {
        strcpy(game->playerName, "Player 1");
    }
}

/* 
 * SYLLABUS TOPIC: RECURSION
 * Recursively counts the number of inversions in a 1D array.
 * An inversion is when a larger number appears before a smaller number.
 * Base Case: when n <= 1, inversions = 0.
 * Recursive step: count how many elements after arr[0] are smaller than arr[0],
 *                 then add countInversionsRecursive(arr + 1, n - 1).
 */
int countInversionsRecursive(int arr[], int n) {
    if (n <= 1) {
        return 0; /* Base case */
    }

    int currentInversions = 0;
    for (int i = 1; i < n; i++) {
        if (arr[0] > arr[i]) {
            currentInversions++;
        }
    }

    /* Recursive call on the remainder of the array */
    return currentInversions + countInversionsRecursive(arr + 1, n - 1);
}

/*
 * SYLLABUS TOPIC: 1D & 2D ARRAYS
 * Flattens the 2D board into a 1D array excluding the blank tile (0),
 * and checks mathematical solvability for a 4x4 grid.
 * Rule for 4x4 15-Puzzle:
 * - If blank is on an even row from bottom (1-based index from bottom: row 2 or 4)
 *   and number of inversions is odd -> solvable.
 * - If blank is on an odd row from bottom (row 1 or 3)
 *   and number of inversions is even -> solvable.
 */
bool isSolvable(int board[SIZE][SIZE]) {
    int flat[SIZE * SIZE - 1];
    int idx = 0;
    int blankRowFromBottom = 0;

    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
            if (board[r][c] == 0) {
                blankRowFromBottom = SIZE - r; /* 1-based row index from bottom */
            } else {
                flat[idx++] = board[r][c];
            }
        }
    }

    int inversions = countInversionsRecursive(flat, idx);

    if (blankRowFromBottom % 2 == 0) {
        return (inversions % 2 != 0);
    } else {
        return (inversions % 2 == 0);
    }
}

/* 
 * Shuffles the board by performing random valid moves starting from the solved state.
 * This guarantees the puzzle is always 100% solvable and engaging.
 */
void shuffleBoard(struct GameState *game) {
    char dirs[4] = {'W', 'A', 'S', 'D'};
    int shuffleMoves = 150;

    srand((unsigned int)time(NULL));

    for (int i = 0; i < shuffleMoves; i++) {
        char randomDir = dirs[rand() % 4];
        moveTile(game, randomDir);
    }

    /* Reset stats after shuffling so move count starts at 0 */
    game->moves = 0;
    game->startTime = time(NULL);
    game->elapsedSeconds = 0;
    game->isWon = false;
}

/*
 * SYLLABUS TOPIC: 2D ARRAYS & LOOPS
 * Intuitive Player Direction Mapping:
 * 'W' / Up Arrow    : Moves the empty space UP (slides tile above the blank down).
 * 'S' / Down Arrow  : Moves the empty space DOWN (slides tile below the blank up).
 * 'A' / Left Arrow  : Moves the empty space LEFT (slides tile to the left of blank right).
 * 'D' / Right Arrow : Moves the empty space RIGHT (slides tile to the right of blank left).
 */
bool moveTile(struct GameState *game, char direction) {
    int targetR = game->emptyRow;
    int targetC = game->emptyCol;

    switch (direction) {
        case 'W':
        case 'w':
            targetR = game->emptyRow - 1; /* Blank moves UP */
            break;
        case 'S':
        case 's':
            targetR = game->emptyRow + 1; /* Blank moves DOWN */
            break;
        case 'A':
        case 'a':
            targetC = game->emptyCol - 1; /* Blank moves LEFT */
            break;
        case 'D':
        case 'd':
            targetC = game->emptyCol + 1; /* Blank moves RIGHT */
            break;
        default:
            return false;
    }

    /* Boundary validation */
    if (targetR < 0 || targetR >= SIZE || targetC < 0 || targetC >= SIZE) {
        return false; /* Invalid move */
    }

    /* Swap target tile with blank tile */
    game->board[game->emptyRow][game->emptyCol] = game->board[targetR][targetC];
    game->board[targetR][targetC] = 0;
    game->emptyRow = targetR;
    game->emptyCol = targetC;
    game->moves++;

    /* Update elapsed time */
    game->elapsedSeconds = (int)(time(NULL) - game->startTime);

    /* Check if move solved the board */
    game->isWon = checkWin(game);
    return true;
}

/*
 * Checks if all tiles 1 to 15 are in sequential order and blank is at the bottom right.
 */
bool checkWin(const struct GameState *game) {
    int expected = 1;
    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
            if (r == SIZE - 1 && c == SIZE - 1) {
                if (game->board[r][c] != 0) return false;
            } else {
                if (game->board[r][c] != expected) return false;
                expected++;
            }
        }
    }
    return true;
}

/*
 * SYLLABUS TOPIC: BASIC SORTING ALGORITHMS (Selection Sort)
 * Sorts the leaderboard array by fewest moves first.
 * If moves are equal, sorts by least time spent.
 */
void sortLeaderboard(struct LeaderboardEntry entries[], int count) {
    for (int i = 0; i < count - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < count; j++) {
            if (entries[j].moves < entries[minIdx].moves) {
                minIdx = j;
            } else if (entries[j].moves == entries[minIdx].moves) {
                if (entries[j].timeSpentSeconds < entries[minIdx].timeSpentSeconds) {
                    minIdx = j;
                }
            }
        }
        if (minIdx != i) {
            /* Swap records */
            struct LeaderboardEntry temp = entries[i];
            entries[i] = entries[minIdx];
            entries[minIdx] = temp;
        }
    }
}

/*
 * SYLLABUS TOPIC: FILE HANDLING
 * Appends a new winner score to leaderboard.txt
 */
bool saveScore(const struct LeaderboardEntry *entry) {
    FILE *fp = fopen(LEADERBOARD_FILE, "a");
    if (!fp) {
        return false;
    }

    fprintf(fp, "%s %d %d %s\n",
            entry->name,
            entry->moves,
            entry->timeSpentSeconds,
            entry->date);

    fclose(fp);
    return true;
}

/*
 * SYLLABUS TOPIC: FILE HANDLING & SORTING
 * Reads all entries from leaderboard.txt into an array and sorts them.
 */
int loadLeaderboard(struct LeaderboardEntry entries[], int maxEntries) {
    FILE *fp = fopen(LEADERBOARD_FILE, "r");
    if (!fp) {
        return 0; /* File does not exist yet */
    }

    int count = 0;
    while (count < maxEntries &&
           fscanf(fp, "%31s %d %d %19s",
                  entries[count].name,
                  &entries[count].moves,
                  &entries[count].timeSpentSeconds,
                  entries[count].date) == 4) {
        count++;
    }

    fclose(fp);

    /* Sort entries using Selection Sort */
    sortLeaderboard(entries, count);

    return count;
}

/*
 * SYLLABUS TOPIC: BINARY FILE HANDLING
 * Saves game state so player can pause and resume later.
 */
bool saveGameProgress(const struct GameState *game) {
    FILE *fp = fopen(SAVE_FILE, "wb");
    if (!fp) return false;

    size_t written = fwrite(game, sizeof(struct GameState), 1, fp);
    fclose(fp);
    return (written == 1);
}

/*
 * Reads saved game state from file.
 */
bool loadGameProgress(struct GameState *game) {
    FILE *fp = fopen(SAVE_FILE, "rb");
    if (!fp) return false;

    size_t read = fread(game, sizeof(struct GameState), 1, fp);
    fclose(fp);

    if (read == 1) {
        /* Re-adjust start time so timer continues accurately */
        game->startTime = time(NULL) - game->elapsedSeconds;
        return true;
    }
    return false;
}

/*
 * Checks if a saved game exists.
 */
bool hasSavedGame(void) {
    FILE *fp = fopen(SAVE_FILE, "rb");
    if (fp) {
        fclose(fp);
        return true;
    }
    return false;
}
