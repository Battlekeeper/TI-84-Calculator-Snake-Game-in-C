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
const int maxSize = 12 * 16;
int snakeLength;
int high_score;

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

void resetSnake(struct snakeBody *snake)
{
    snakeLength = 3;
    for (int i = 0; i < maxSize; i++)
    {
        snake[i].x = -1;
        snake[i].y = -1;
    }
    snake[0].x = 16;
    snake[0].y = 12;
    snake[0].dir = RIGHT;

    snake[1].x = 15;
    snake[1].y = 12;
    snake[1].dir = RIGHT;

    snake[2].x = 14;
    snake[2].y = 12;
    snake[2].dir = RIGHT;
}
void moveApple(struct apple *apple, struct snakeBody *snake)
{
    apple->x = randInt(0, 31);
    apple->y = randInt(0, 23);
    for (int i = 0; i < maxSize; i++)
    {
        if (snake[i].x == apple->x && snake[i].y == apple->y)
        {
            moveApple(apple, snake);
        }
    }
    dbg_printf("Apple: %d, %d\n", apple->x, apple->y);

}

void getInput(struct snakeBody *snake)
{
    kb_Scan();
    kb_key_t key = kb_Data[7];

    switch (key)
    {
    case kb_Down:
        if (snake[0].dir != UP)
        {
            snake[0].dir = DOWN;
        }
        break;

    case kb_Right:
        if (snake[0].dir != LEFT)
        {
            snake[0].dir = RIGHT;
        }
        break;

    case kb_Up:
        if (snake[0].dir != DOWN)
        {
            snake[0].dir = UP;
        }
        break;

    case kb_Left:
        if (snake[0].dir != RIGHT)
        {
            snake[0].dir = LEFT;
        }
        break;
    default:
        break;
    }
}

bool moveSnake(struct snakeBody *snake)
{
    for (int i = snakeLength; i >= 0; i--)
    {
        if (snake[i].x == -1)
        {
            continue;
        }
        if (snake[i].dir == UP)
        {
            snake[i].y--;
        }
        else if (snake[i].dir == DOWN)
        {
            snake[i].y++;
        }
        else if (snake[i].dir == LEFT)
        {
            snake[i].x--;
        }
        else if (snake[i].dir == RIGHT)
        {
            snake[i].x++;
        }
        else if (snake[i].dir == STOPPED)
        {
            snake[i].dir = snake[i - 1].dir;
        }
        if (i > 0)
        {
            snake[i].dir = snake[i - 1].dir;
        }
    }
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
        moveApple(apple, snake);
    }
}

bool gameEnd(struct snakeBody *snake, struct apple *apple)
{
    if (snakeLength - 3 > high_score)
    {
        high_score = snakeLength - 3;
        uint8_t high_score_handle = ti_Open("high_score", "w");
        ti_Write(&high_score, sizeof(high_score), 1, high_score_handle);
        ti_Close(high_score_handle);
    }


    char score[20];
    sprintf(score, "Score: %d", snakeLength - 3);
    char high_score_str[20];
    sprintf(high_score_str, "High Score: %d", high_score);

    gfx_PrintStringXY(score, 320 / 2 - 25, 240 / 2 - 5);
    gfx_PrintStringXY(high_score_str, 320 / 2 - 35, 240 / 2 + 5);

    gfx_PrintStringXY("Press any key to continue", 320 / 2 - 75, 240 / 2 + 80);
    gfx_PrintStringXY("Press clear to quit", 320 / 2 - 55, 240 / 2 + 100);

    gfx_BlitBuffer();
    uint8_t key = 0;

    while (!key){
        key = os_GetCSC();
    }
    resetSnake(snake);
    moveApple(apple, snake);
    return key == sk_Clear;
}

int main(void)
{
    timer_Enable(2, TIMER_32K, TIMER_NOINT, TIMER_UP);
    //Init Random
    srandom(timer_Get(2));

    // Initialize the graphics
    gfx_Begin();
    gfx_SetTextTransparentColor(248);
    gfx_SetDrawBuffer();
    gfx_ZeroScreen();
    gfx_BlitBuffer();

    uint8_t high_score_handle = ti_Open("high_score", "r");
    dbg_printf("Handle: %d\n", high_score_handle);

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

    char high_score_str[20]; // Assuming the high score can be represented in 10 characters or less
    sprintf(high_score_str, "High Score: %d", high_score);

    gfx_SetTextFGColor(255);
    gfx_SetTextBGColor(0);

    gfx_PrintStringXY(high_score_str, 320 / 2 - 50, 240 / 2 - 10);
    gfx_PrintStringXY("Press any key to start", 320 / 2 - 70, 240 / 2 + 100);

    gfx_BlitBuffer();
    while (!os_GetCSC()){}

    timer_Enable(1, TIMER_32K, TIMER_NOINT, TIMER_UP);

    struct snakeBody *snake = (struct snakeBody *)malloc(maxSize * sizeof(struct snakeBody));

    struct apple apple;
    resetSnake(snake);
    moveApple(&apple, snake);
    handleApple(snake, &apple);

    bool end = false;

    while (os_GetCSC() != sk_Clear && !end)
    {
        srandom(timer_Get(2));
        timer_Set(1, 0);
        gfx_ZeroScreen();
        gfx_SetColor(1);
        for (int i = 0; i < 32; i++)
        {
            gfx_Line(i * 10, 0, i * 10, 240);
        }
        for (int i = 0; i < 24; i++)
        {
            gfx_Line(0, i * 10, 320, i * 10);
        }

        getInput(snake);
        handleApple(snake, &apple);
        gfx_SetColor(224);
        gfx_FillCircle(apple.x * 10 + 5, apple.y * 10 + 5, 4);
        bool crashed = moveSnake(snake);

        gfx_SetColor(0x07);
        for (int i = 0; i < snakeLength; i++)
        {
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

        // Draw Border
        gfx_SetColor(255);
        gfx_Rectangle(0, 0, 320, 240);
        
        if (snake[0].x > 31 || snake[0].x < 0 || snake[0].y > 23 || snake[0].y < 0 || crashed)
        {
            end = gameEnd(snake, &apple);
        }

        int score = snakeLength - 3;
        char score_str[20];
        sprintf(score_str, "%d", score);

        gfx_PrintStringXY(score_str, 0,0);
        gfx_BlitBuffer();
        while (timer_Get(1) < 2300)
        {
            getInput(snake);
        }
    }
    free(snake);
    gfx_End();
    return 0;
}
