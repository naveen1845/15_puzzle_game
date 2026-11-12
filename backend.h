#ifndef BACKEND_H
#define BACKEND_H

#include <stdbool.h>
#include <time.h>

#define SIZE 4
#define MAX_NAME_LEN 32
#define MAX_LEADERBOARD_ENTRIES 50
#define LEADERBOARD_FILE "leaderboard.txt"
#define SAVE_FILE "saved_game.dat"

/* Structure to store a single player's score record (Demonstrates Structures) */
struct LeaderboardEntry {
    char name[MAX_NAME_LEN];
    int moves;
    int timeSpentSeconds;
    char date[20];
};

/* Structure to hold complete game state (Demonstrates Structures & 2D Arrays) */
struct GameState {
    int board[SIZE][SIZE];       /* 2D Array: 1-15 tiles, 0 represents blank */
    int emptyRow;                /* Current row of blank tile */
    int emptyCol;                /* Current column of blank tile */
    int moves;                   /* Total valid moves made */
    time_t startTime;            /* Game start timestamp */
    int elapsedSeconds;          /* Time elapsed */
    bool isWon;                  /* Flag indicating win condition */
    char playerName[MAX_NAME_LEN];
};

/* Core Backend Functions */
void initBoard(struct GameState *game, const char *playerName);
void shuffleBoard(struct GameState *game);
bool moveTile(struct GameState *game, char direction); /* 'W', 'A', 'S', 'D' */
bool checkWin(const struct GameState *game);

/* Recursion: Solvability Check (Demonstrates Recursion & 1D Array flattening) */
int countInversionsRecursive(int arr[], int n);
bool isSolvable(int board[SIZE][SIZE]);

/* File Handling & Sorting (Demonstrates File I/O and Selection Sort) */
bool saveScore(const struct LeaderboardEntry *entry);
int loadLeaderboard(struct LeaderboardEntry entries[], int maxEntries);
void sortLeaderboard(struct LeaderboardEntry entries[], int count);

/* Game Persistence */
bool saveGameProgress(const struct GameState *game);
bool loadGameProgress(struct GameState *game);
bool hasSavedGame(void);

#endif /* BACKEND_H */
