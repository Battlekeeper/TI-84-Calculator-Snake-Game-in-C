#include <ti/getcsc.h>
#include <graphx.h>
#include <stdlib.h>
#include <debug.h>
#include <time.h>
#include <keypadc.h>
#include <sys/timers.h>
#include <fileioc.h>

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
struct apple
{
    int x;
    int y;
};
const int maxSize = 12 * 16;
int snakeLength;
int high_score;

void resetSnake(struct snakeBody *snake)
{
    // Reset the snake to the starting position
    snakeLength = 3;
    for (int i = 0; i < maxSize; i++)
    {
        snake[i].x = -1;
        snake[i].y = -1;
    }
    snake[0].x = 16;
    snake[0].y = 12;
    snake[0].dir = randInt(0, 3);

    snake[1].x = 16;
    snake[1].y = 12;
    snake[1].dir = STOPPED;

    snake[2].x = 16;
    snake[2].y = 12;
    snake[2].dir = STOPPED;
}

void moveApple(struct apple *apple, struct snakeBody *snake)
{
    apple->x = randInt(0, 31);
    apple->y = randInt(0, 23);
    for (int i = 0; i < snakeLength; i++)
    {
        // Make sure the apple doesn't spawn on the snake
        // This is recursive until a valid location is found
        if (snake[i].x == apple->x && snake[i].y == apple->y)
        {
            moveApple(apple, snake);
        }
    }
}

void handleInput(struct snakeBody *snake)
{
    kb_key_t key = kb_Data[7];

    switch (key)
    {
    case kb_Down:
        snake[0].dir = DOWN;
        break;

    case kb_Right:
        snake[0].dir = RIGHT;
        break;
    case kb_Up:
        snake[0].dir = UP;
        break;

    case kb_Left:
        snake[0].dir = LEFT;
        break;
    default:
        break;
    }
}

bool moveSnake(struct snakeBody *snake)
{
    // This loop moved backwards so that the snake body follows the head
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

    // Check if the snake has crashed into itself
    for (int i = 1; i < snakeLength; i++)
    {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y)
        {
            return true;
        }
    }
    return false;
}

void handleApple(struct snakeBody *snake, struct apple *apple)
{

    if (snake[0].x == apple->x && snake[0].y == apple->y)
    {
        snake[snakeLength].x = snake[snakeLength - 1].x;
        snake[snakeLength].y = snake[snakeLength - 1].y;
        snake[snakeLength].dir = STOPPED;
        snakeLength++;
        timer_Set(3, 0);
        moveApple(apple, snake);
    }
    else
    {
        if (timer_Get(3) >= 288000)
        {
            moveApple(apple, snake);
            timer_Set(3, 0);
        }
    }
}

bool gameEnd(struct snakeBody *snake, struct apple *apple, bool win)
{
    if (snakeLength - 3 > high_score)
    {
        high_score = snakeLength - 3;
        uint8_t high_score_handle = ti_Open("high_score", "w");
        ti_Write(&high_score, sizeof(high_score), 1, high_score_handle);
        ti_Close(high_score_handle);
    }

    if (win)
    {
        gfx_SetColor(7);
        gfx_SetTextFGColor(7);
        gfx_SetTextScale(2, 2);
        gfx_PrintStringXY("You Win!", 320 / 2 - 40, 240 / 2 - 60);
        gfx_Rectangle(0, 0, 320, 240);
        gfx_SetTextScale(1, 1);
        gfx_SetTextFGColor(255);
    }
    else
    {
        gfx_SetColor(192);
        gfx_SetTextFGColor(192);
        gfx_SetTextScale(2, 2);
        gfx_PrintStringXY("Game Over!", 320 / 2 - 60, 240 / 2 - 60);
        gfx_Rectangle(0, 0, 320, 240);
        gfx_SetTextScale(1, 1);
        gfx_SetTextFGColor(255);
    }

    char score[20];
    sprintf(score, "Score: %d", snakeLength - 3);
    char high_score_str[20];
    sprintf(high_score_str, "High Score: %d", high_score);

    gfx_PrintStringXY(score, 320 / 2 - 25, 240 / 2 - 5);
    gfx_PrintStringXY(high_score_str, 320 / 2 - 35, 240 / 2 + 5);

    gfx_PrintStringXY("Press any key to continue", 320 / 2 - 75, 240 / 2 + 80);
    gfx_PrintStringXY("Press CLEAR to quit", 320 / 2 - 55, 240 / 2 + 100);

    gfx_BlitBuffer();
    uint8_t key = 0;

    delay(500);
    while (!key)
    {
        key = os_GetCSC();
    }
    resetSnake(snake);
    moveApple(apple, snake);

    // Quit game if clear is pressed
    return key == sk_Clear;
}

int main(void)
{
    timer_Enable(1, TIMER_32K, TIMER_NOINT, TIMER_UP);
    timer_Enable(2, TIMER_32K, TIMER_NOINT, TIMER_UP);
    timer_Enable(3, TIMER_32K, TIMER_NOINT, TIMER_UP);

    // Init Random
    srandom(timer_Get(2));

    // Initialize the graphics
    gfx_Begin();
    gfx_SetTextTransparentColor(248);
    gfx_SetDrawBuffer();
    gfx_ZeroScreen();
    gfx_BlitBuffer();

    // Load high score
    uint8_t high_score_handle = ti_Open("high_score", "r");

    if (high_score_handle == 0)
    {
        high_score_handle = ti_Open("high_score", "w");
        int data = 0;
        ti_Write(&data, sizeof(data), 1, high_score_handle);
        ti_Close(high_score_handle);
    }
    high_score_handle = ti_Open("high_score", "r");
    ti_Read(&high_score, sizeof(high_score), 1, high_score_handle);
    ti_Close(high_score_handle);

    char high_score_str[20];
    sprintf(high_score_str, "High Score: %d", high_score);

    gfx_SetTextFGColor(255);
    gfx_SetTextBGColor(0);

    gfx_PrintStringXY(high_score_str, 320 / 2 - 50, 240 / 2 - 10);
    gfx_PrintStringXY("Press any key to start", 320 / 2 - 70, 240 / 2 + 100);

    gfx_SetTextFGColor(3);
    gfx_SetTextScale(2, 2);
    gfx_PrintStringXY("Snake", 320 / 2 - 50, 240 / 2 - 30);
    gfx_SetTextScale(1, 1);
    gfx_SetTextFGColor(255);

    // Wait to start the game
    gfx_BlitBuffer();
    while (!os_GetCSC())
    {
    }

    // Set random seed based on timer value
    srandom(timer_Get(2));

    // Initialize the snake
    struct snakeBody *snake = (struct snakeBody *)malloc(maxSize * sizeof(struct snakeBody));
    struct apple apple;
    resetSnake(snake);
    moveApple(&apple, snake);
    gfx_BlitBuffer();
    bool endGame = false;

    // Clear Apple Timer
    timer_Set(3, 0);

    // Start Game Loop
    while (os_GetCSC() != sk_Clear && !endGame)
    {
        timer_Set(1, 0);

        gfx_ZeroScreen();
        gfx_SetColor(1);

        // Draw screen grid
        for (int i = 0; i < 32; i++)
        {
            gfx_Line(i * 10, 0, i * 10, 240);
        }
        for (int i = 0; i < 24; i++)
        {
            gfx_Line(0, i * 10, 320, i * 10);
        }

        handleInput(snake);
        bool crashed = moveSnake(snake);
        handleApple(snake, &apple);
        gfx_SetColor(224);
        // Draw apple
        gfx_FillCircle(apple.x * 10 + 5, apple.y * 10 + 7, 3);

        // Draw apple stem
        gfx_SetColor(3);
        gfx_SetPixel(apple.x * 10 + 4, apple.y * 10 + 4);
        gfx_SetPixel(apple.x * 10 + 5, apple.y * 10 + 4);
        gfx_SetPixel(apple.x * 10 + 6, apple.y * 10 + 4);

        gfx_SetPixel(apple.x * 10 + 3, apple.y * 10 + 3);
        gfx_SetPixel(apple.x * 10 + 4, apple.y * 10 + 3);
        gfx_SetPixel(apple.x * 10 + 5, apple.y * 10 + 3);

        gfx_SetPixel(apple.x * 10 + 2, apple.y * 10 + 2);
        gfx_SetPixel(apple.x * 10 + 3, apple.y * 10 + 2);

        // Draw snake body
        for (int i = 0; i < snakeLength; i++)
        {
            gfx_SetColor(7 - i % 2 * 4); // Uses modulus to alternate color of the snake body
            gfx_FillRectangle(snake[i].x * 10 + 1, snake[i].y * 10 + 1, 8, 8);
        }

        // Draw Head of snake
        gfx_SetColor(224);
        switch (snake[0].dir)
        {
        case UP:
            gfx_FillRectangle(snake[0].x * 10 + 2, snake[0].y * 10 + 2, 2, 2);
            gfx_FillRectangle(snake[0].x * 10 + 6, snake[0].y * 10 + 2, 2, 2);
            break;
        case DOWN:
            gfx_FillRectangle(snake[0].x * 10 + 2, snake[0].y * 10 + 6, 2, 2);
            gfx_FillRectangle(snake[0].x * 10 + 6, snake[0].y * 10 + 6, 2, 2);
            break;
        case LEFT:
            gfx_FillRectangle(snake[0].x * 10 + 2, snake[0].y * 10 + 2, 2, 2);
            gfx_FillRectangle(snake[0].x * 10 + 2, snake[0].y * 10 + 6, 2, 2);
            break;
        case RIGHT:
            gfx_FillRectangle(snake[0].x * 10 + 6, snake[0].y * 10 + 2, 2, 2);
            gfx_FillRectangle(snake[0].x * 10 + 6, snake[0].y * 10 + 6, 2, 2);
            break;
        }

        // Draw Screen Border
        gfx_SetColor(255);
        gfx_Rectangle(0, 0, 320, 240);

        if (snake[0].x > 31 || snake[0].x < 0 || snake[0].y > 23 || snake[0].y < 0 || crashed)
        {
            endGame = gameEnd(snake, &apple, false);
            timer_Set(3, 0);
        }
        if (snakeLength == 56)
        {
            endGame = gameEnd(snake, &apple, true);
            timer_Set(3, 0);
        }

        int score = snakeLength - 3;
        char score_str[20];
        sprintf(score_str, "%d", score);

        gfx_PrintStringXY(score_str, 0, 0);
        gfx_BlitBuffer();

        do
        {
            if (kb_Data[7] != kb_Right && kb_Data[7] != kb_Left && kb_Data[7] != kb_Up && kb_Data[7] != kb_Down)
            {
                kb_Scan();
            }
        } while (timer_Get(1) < 4000 - (snakeLength * 35)); // Sets speed of snake based on length
    }
    free(snake);
    gfx_End();
    return 0;
}
