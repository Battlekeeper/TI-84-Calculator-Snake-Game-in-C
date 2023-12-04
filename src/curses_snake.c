#include <curses.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

enum direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    STOPPED
};
struct snakeBody
{
    int x;
    int y;
    int dir;
};
struct trophy
{
    int x;
    int y;
    int value;
    int expireTime;
};

int screenMaxX, screenMaxY;
int snakeLength = 3;
int snakeMaxSize;
time_t trophyStartTime, trophyCurrentTime;

void drawBorder()
{
    int i;
    mvprintw(0, 0, "+");
    mvprintw(0, screenMaxX - 1, "+");
    mvprintw(screenMaxY - 1, 0, "+");
    mvprintw(screenMaxY - 1, screenMaxX - 1, "+");

    for (i = 1; i < (screenMaxY - 1); i++)
    {
        mvprintw(i, 0, "|");
        mvprintw(i, screenMaxX - 1, "|");
    }
    for (i = 1; i < (screenMaxX - 1); i++)
    {
        mvprintw(0, i, "-");
        mvprintw(screenMaxY - 1, i, "-");
    }
}

void resetSnake(struct snakeBody *snake)
{
    snakeLength = 3;
    for (int i = 0; i < snakeMaxSize; i++)
    {
        snake[i].x = -1;
        snake[i].y = -1;
    }
    snake[0].x = screenMaxX / 2;
    snake[0].y = screenMaxY / 2;
    snake[0].dir = rand() % 4;

    snake[1].x = screenMaxX / 2;
    snake[1].y = screenMaxY / 2;
    snake[1].dir = STOPPED;

    snake[2].x = screenMaxX / 2;
    snake[2].y = screenMaxY / 2;
    snake[2].dir = STOPPED;
}

void resetTrophy(struct snakeBody *snake, struct trophy *trophy)
{
    trophy->x = rand() % (screenMaxX - 2) + 1;
    trophy->y = rand() % (screenMaxY - 2) + 1;
    for (int i = 0; i < snakeLength; i++)
    {
        if (trophy->x == snake[i].x && trophy->y == snake[i].y)
        {
            resetTrophy(snake, trophy);
        }
    }
    trophy->value = rand() % 9 + 1;
    trophy->expireTime = rand() % 9 + 1;
    time(&trophyStartTime);
}

void winScreen()
{
    clear();
    mvprintw(screenMaxY / 2, screenMaxX / 2 - 4, "You win!");
    mvprintw(screenMaxY / 2 + 1, screenMaxX / 2 - 14, "Press any key to continue...");
    move(screenMaxY - 1, screenMaxX - 1);

    refresh();
    nodelay(stdscr, FALSE);
    usleep(500000);
    getch();
    clear();
    refresh();
    nodelay(stdscr, TRUE);
}

void handleTrophy(struct snakeBody *snake, struct trophy *trophy)
{
    time(&trophyCurrentTime);

    if (difftime(trophyCurrentTime, trophyStartTime) >= trophy->expireTime)
    {
        resetTrophy(snake, trophy);
    }
    if (snake[0].x == trophy->x && snake[0].y == trophy->y)
    {
        int last = snakeLength - 1;
        for (int i = 0; i < trophy->value; i++)
        {
            snakeLength++;
            snake[snakeLength - 1].x = snake[last].x;
            snake[snakeLength - 1].y = snake[last].y;
            snake[snakeLength - 1].dir = STOPPED;
            if (snakeMaxSize == snakeLength)
            {
                winScreen();
                resetSnake(snake);
                resetTrophy(snake, trophy);
            }
        }

        resetTrophy(snake, trophy);
    }
}

void validateGameState(struct snakeBody *snake, struct trophy *trophy)
{
    // Check for collisions
    if (snake[0].x == 0 || snake[0].x == screenMaxX - 1 || snake[0].y == 0 || snake[0].y == screenMaxY - 1)
    {
        resetSnake(snake);
        resetTrophy(snake, trophy);
    }
    for (int i = 1; i < snakeLength; i++)
    {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y)
        {
            resetSnake(snake);
            resetTrophy(snake, trophy);
        }
    }
}

void moveSnake(struct snakeBody *snake)
{
    for (int i = snakeLength; i >= 0; i--)
    {
        if (snake[i].x == -1)
        {
            continue;
        }

        switch (snake[i].dir)
        {
        case UP:
            snake[i].y--;
            break;
        case DOWN:
            snake[i].y++;
            break;
        case LEFT:
            snake[i].x--;
            break;
        case RIGHT:
            snake[i].x++;
            break;
        case STOPPED:
            snake[i].dir = snake[i - 1].dir;
            break;
        }

        if (i > 0)
        {
            snake[i].dir = snake[i - 1].dir;
        }
    }
}

int main()
{
    // Initialize curses
    srand(time(NULL));
    initscr();
    cbreak();
    curs_set(0);
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    getmaxyx(stdscr, screenMaxY, screenMaxX);

    // Initialize game state
    drawBorder();
    snakeMaxSize = (screenMaxY * 2 + screenMaxX * 2) / 2;
    struct snakeBody *snake = (struct snakeBody *)malloc(snakeMaxSize * sizeof(struct snakeBody));
    struct trophy *trophy = (struct trophy *)malloc(sizeof(struct trophy));
    resetSnake(snake);
    resetTrophy(snake, trophy);

    // Main game loop
    int ch;
    while (ch != 'q')
    {
        // Clear the screen and add border
        clear();
        drawBorder();

        // Update game state
        moveSnake(snake);
        validateGameState(snake, trophy);
        handleTrophy(snake, trophy);

        // Draw the trophy
        mvprintw(trophy->y, trophy->x, "%d", trophy->value);

        // Draw the snake
        for (int i = 0; i < snakeLength; i++)
        {
            if (snake[i].x == -1)
            {
                continue;
            }
            mvprintw(snake[i].y, snake[i].x, "o");
        }

        mvprintw(0, 1, "Score: %d   Win Score: %d", snakeLength, snakeMaxSize);
        // Update UI
        refresh();
        move(screenMaxY - 1, screenMaxX - 1);

        for (int i = 0; i < 750 - (500 / snakeMaxSize) * snakeLength; i++)
        {
            usleep(100);
            int temp = getch();
            if (temp == 'q')
            {
                ch = 'q';
                break;
            }
            else if (temp == KEY_UP)
            {
                snake[0].dir = UP;
            }
            else if (temp == KEY_DOWN)
            {
                snake[0].dir = DOWN;
            }
            else if (temp == KEY_LEFT)
            {
                snake[0].dir = LEFT;
            }
            else if (temp == KEY_RIGHT)
            {
                snake[0].dir = RIGHT;
            }
        }
    }
    endwin();
    free(snake);
    return 0;
}