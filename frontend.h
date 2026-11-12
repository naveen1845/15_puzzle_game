#ifndef FRONTEND_H
#define FRONTEND_H

#include "backend.h"

/* ANSI Color & Style Codes */
#define ANSI_RESET       "\033[0m"
#define ANSI_BOLD        "\033[1m"
#define ANSI_DIM         "\033[2m"
#define ANSI_RED         "\033[31m"
#define ANSI_GREEN       "\033[32m"
#define ANSI_YELLOW      "\033[33m"
#define ANSI_BLUE        "\033[34m"
#define ANSI_MAGENTA     "\033[35m"
#define ANSI_CYAN        "\033[36m"
#define ANSI_WHITE       "\033[37m"
#define ANSI_BG_BLUE     "\033[44m"
#define ANSI_BG_DARK     "\033[48;5;236m"

/* Screen clearing and cursor control */
void clearScreen(void);

/* Terminal input configuration (Immediate keystroke without pressing Enter) */
void enableRawMode(void);
void disableRawMode(void);
int getKeyPress(void);

/* UI Views / Screen Renders */
void renderBanner(void);
void renderMenu(bool canResume);
void renderBoard(const struct GameState *game);
void renderLeaderboard(const struct LeaderboardEntry entries[], int count);
void renderHelp(void);
void renderWinScreen(const struct GameState *game);
void renderMessage(const char *msg, const char *color);

#endif /* FRONTEND_H */
