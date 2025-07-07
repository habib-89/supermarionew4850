#include "iGraphics.h"
#include <iostream>
#include "iSound.h"
using namespace std;

#define MENU 0
#define GAME 1
#define GAME_OVER 2
#define FRONT_PAGE -1
#define HELP 3
#define LEVEL_SELECT 4
#define MAP_WIDTH 68
#define MAP_HEIGHT 17
#define GROUND 122

int golem_height = 76;
int golem_width = 45;

int pic_x, pic_y;
int golem_idx = 0;
int jump_idx = 0;
// char golem_idle[17][100];
// char golem_jump[11][100];
int jump = 0;
int jump_speed = 0;
int gravity = 1;
int jump_height = 15;
int ground = GROUND;
int direction = 0;
int tile_x = 0, tile_y = ground;
int tile_width = 30, tile_height = 30;
Image bg[4];
Image golem_idle[2];
Image golem_run_frames[7];
Image golem_jump_frames[11];
Image tile_set[2];
Sprite tiles[MAP_HEIGHT * MAP_WIDTH];
Sprite golem;
int speed = 0;
int golem_running = 0;
int animation = -1; // 0 idle, 1 run, 2 jump
bool going_right = true;
bool gameOver = false;
bool isPaused = false;
bool touch = false;
;
int bgSoundIdx = -1;
char tileMap[MAP_HEIGHT][MAP_WIDTH];
char tile_type[MAP_HEIGHT * MAP_WIDTH];
int gameState = FRONT_PAGE;
int gameStartTime = 0;

const int bgScrollSpeed = 2;

int tile_idx = 0;
int scroll_x = 0;

bool hasPreviousGame = false;
int prev_pic_x, prev_pic_y;
int prev_jump, prev_jumpSpeed;
bool prev_gameOver;
int prev_gameStartTime;

int currentLevel = 1;
const int maxLevel = 3;

void load_bg()
{
    // Load backgrounds for 3 levels   Level1BG.png
    iLoadImage(&bg[1], "assets/Level1image/BGL1001.png");
    iLoadImage(&bg[2], "Game Project Pic/Level2BG.jpg");
    iLoadImage(&bg[3], "Game Project Pic/Level3BG.jpg");

    // Resize all to fit screen
    for (int i = 1; i <= 3; i++)
    {
        iResizeImage(&bg[i], 800, 500);
    }
}

void loadLevelFromFile(int level)
{
    char filename[100];
    sprintf(filename, "levels(rafsan)/level_%d.txt", level);

    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Failed to open %s\n", filename);
        return;
    }

    for (int i = 0; i < MAP_HEIGHT; i++)
    {
        for (int j = 0; j < MAP_WIDTH; j++)
        {
            fscanf(fp, "%c", &tileMap[i][j]);
        }
    }

    fclose(fp);

    iLoadImage(&tile_set[0], "assets/Level1image/Brick_01.png"); // brick
    iLoadImage(&tile_set[1], "assets/Level1image/Coin003.png");  // coin

    tile_idx = 0;
    tile_x = 0;
    const int h = tile_height * (MAP_HEIGHT - 1);
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        tile_y = h - tile_height * y;
        tile_x = 0;
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            if (tileMap[y][x] == '*')
            {
                iInitSprite(&tiles[tile_idx], -1);
                iChangeSpriteFrames(&tiles[tile_idx], &tile_set[0], 1);
                iSetSpritePosition(&tiles[tile_idx], tile_x, tile_y);
                tile_x += tile_width;
                tile_type[tile_idx] = '*';
                tile_idx++;
            }
            else if (tileMap[y][x] == 'o')
            {
                iInitSprite(&tiles[tile_idx], -1);
                iChangeSpriteFrames(&tiles[tile_idx], &tile_set[1], 1);
                iSetSpritePosition(&tiles[tile_idx], tile_x, tile_y);
                tile_x += tile_width;
                tile_type[tile_idx] = 'o';
                tile_idx++;
            }
            else if (tileMap[y][x] == '_')
                tile_x += tile_width;
        }
    }
}

void loadResources()
{
    iLoadFramesFromFolder(golem_idle, "Game Project Pic/walk");
    iLoadFramesFromFolder(golem_run_frames, "Game Project Pic/run");
    iLoadFramesFromFolder(golem_jump_frames, "Game Project Pic/jump");
    iInitSprite(&golem, -1);
    iChangeSpriteFrames(&golem, golem_idle, 1);
    iSetSpritePosition(&golem, 70, 122);
}

void saveGameState()
{
    prev_pic_x = pic_x;
    prev_pic_y = pic_y;
    prev_jump = jump;
    // prev_jumpSpeed = jumpSpeed;
    // prev_bgX = bgX;
    prev_gameOver = gameOver;
    prev_gameStartTime = gameStartTime;
    hasPreviousGame = true;
}
void loadGameState()
{
    if (!hasPreviousGame)
        return;

    pic_x = prev_pic_x;
    pic_y = prev_pic_y;
    jump = prev_jump;
    // jumpSpeed = prev_jumpSpeed;
    //  bgX = prev_bgX;
    gameOver = prev_gameOver;
    gameStartTime = prev_gameStartTime;
}

void activity(int current_state)
{
    if (animation != current_state)
    {
        animation = current_state;
        if (current_state == 0)
            iChangeSpriteFrames(&golem, golem_idle, 1);
        if (current_state == 1)
            iChangeSpriteFrames(&golem, golem_run_frames, 7);
        if (current_state == 2)
            iChangeSpriteFrames(&golem, golem_jump_frames, 10);
    }
}

int collision_idx(Sprite *s)
{
    // tiles[i].x>0 && tiles[i].x<800
    for (int i = 0; i < tile_idx; i++)
    {
        // golem.y>= ground &&
        if (iCheckCollision(s, &tiles[i]))
        {
            return i;
        }
    }
    return -1;
}

void update_jump()
{
    Sprite test = golem;
    int idx;

    if (jump)
    {
        golem.y += jump_speed;

        if (direction == 1 && golem.x < 350)
        {
            test.x += 3;
            if (collision_idx(&test) == -1)
            {
                golem.x += 3;
            }
            activity(2);
        }
        else if (direction == 1 && golem.x >= 350)
        {
            speed = -3;
        }
        else if (direction == -1)
        {
            test.x -= 3;
            if (collision_idx(&test) == -1)
            {
                golem.x -= 3;
            }
            activity(2);
        }

        jump_speed -= gravity;

        if (golem.y <= ground)
        {
            golem.y = ground;
            jump = 0;
            jump_speed = 0;
            activity(1);
        }
    }
}


void iSpecialKeyboard(unsigned char key)
{
    if (gameState == GAME)
    {
        if (key == GLUT_KEY_END)
        {
            printf("%d %d", golem.x, golem.y);
            exit(0);
        }
        if (key == GLUT_KEY_LEFT)
        {
            direction = -1;
            if (going_right)
            {
                iMirrorSprite(&golem, HORIZONTAL);
                going_right = false;
            }
        }
        if (key == GLUT_KEY_RIGHT)
        {
            direction = 1;
            if (!going_right)
            {
                iMirrorSprite(&golem, HORIZONTAL);
                going_right = true;
            }
        }
        if (key == GLUT_KEY_UP)
        {
            if (!jump)
            {
                jump = 1;
                jump_speed = jump_height;
                activity(2);
            }
        }
        if (key == GLUT_KEY_DOWN)
        {
            direction = 0;
            speed = 0;
        }
    }
}
void iDraw()
{
    iClear();
    if (gameState == FRONT_PAGE)
    {
        iShowImage(0, 0, "Game Project Pic/1st Cover003.bmp");
        iSetColor(0, 0, 0);
        iText(10, 10, "Press Enter to Continue or Click Main Menu", GLUT_BITMAP_HELVETICA_18);
        return;
    }

    else if (gameState == MENU)
    {
        iShowImage(0, 0, "Game Project Pic/2nd cover003.png");

        iSetColor(0, 0, 0);
        iText(10, 10, "Press e to Exit or Click Exit button.", GLUT_BITMAP_HELVETICA_18);
        iSetColor(0, 0, 0);
        iText(10, 40, "Press New game to start the game.", GLUT_BITMAP_HELVETICA_18);
    }

    else if (gameState == HELP)
    {
        iClear();

        iShowImage(0, 0, "Game Project Pic/Help Cover001.png");

        iSetColor(0, 0, 0);
        iText(300, 450, "HELP", GLUT_BITMAP_TIMES_ROMAN_24);

        iText(100, 310, "1. Use RIGHT and LEFT arrow keys to move");
        iText(100, 280, "2. Press UP arrow key to jump");
        iText(100, 280, "3. Press DOWN arrow key to stop");
        iText(100, 250, "4. Avoid obstacles to survive");
        iText(100, 220, "5. Press 'R' to restart the game after Game Over");
        iText(100, 190, "6. Press 'E' to exit the game");
        // Back Button
        iSetColor(100, 100, 100);
        iFilledRectangle(230, 20, 100, 35);
        iSetColor(255, 255, 255);
        iText(265, 35, "Back");
    }

    else if (gameState == LEVEL_SELECT)
    {
        iClear();
        iShowImage(0, 0, "Game Project Pic/Level BG001.png"); // optional background
    }

    else if (gameState == GAME)
    {
        iShowLoadedImage(0, 0, &bg[currentLevel]);
        iWrapImage(&bg[1], speed);
        iShowSprite(&golem);
        iText(70, 30, "Press P to pause and resume.", GLUT_BITMAP_TIMES_ROMAN_24);
        iText(70, 10, "Press M to go to Menu.", GLUT_BITMAP_TIMES_ROMAN_24);
        if (gameOver)
        {
            iSetColor(255, 0, 0);
            iText(300, 250, "Game Over! Press R to Restart", GLUT_BITMAP_HELVETICA_18);
        }
        // new code
        for (int i = 0; i < tile_idx; i++)
        {
            if (tile_type[i] != '_')
            {
                iShowSprite(&tiles[i]);
            }
        }
        //
    }
}

/*
function iMouseMove() is called when the user moves the mouse.
(mx, my) is the position where the mouse pointer is.
*/
void iMouseMove(int mx, int my)
{
    // place your codes here
}

/*
function iMouseDrag() is called when the user presses and drags the mouse.
(mx, my) is the position where the mouse pointer is.
*/
void iMouseDrag(int mx, int my)
{
    // place your codes here
}

/*
function iMouse() is called when the user presses/releases the mouse.
(mx, my) is the position where the mouse pointer is.
*/
void iMouse(int button, int state, int mx, int my)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        if (gameState == FRONT_PAGE)
        {
            if (mx >= 455 && mx <= 755 && my >= 20 && my <= 70)
            {
                gameState = MENU;
            }
        }

        else if (gameState == MENU)
        {
            /*
            // New Game
            if (mx >= 208 && mx <= 596 && my >= 330 && my <= 385)
            {
                gameState = GAME;
                gameOver = false;
                gameStartTime = time(NULL);
            }
                */
            // New Game
            if (mx >= 208 && mx <= 596 && my >= 330 && my <= 385)
            {
                gameState = LEVEL_SELECT; // <-- go to new screen instead of GAME
            }
            // Score

            // Help
            else if (mx >= 208 && mx <= 596 && my >= 114 && my <= 165)
            {
                gameState = HELP;
            }
            // Exit
            else if (mx >= 208 && mx <= 596 && my >= 66 && my <= 112)
            {
                exit(0);
            }

            // previous game
            if (mx >= 208 && mx <= 596 && my >= 270 && my <= 320)
            {
                if (hasPreviousGame)
                {
                    loadGameState();
                    gameState = GAME;
                }
                else
                {
                    printf("No previous game saved.\n");
                }
            }
        }

        else if (gameState == LEVEL_SELECT)
        {
            // Level-1 button
            if (mx >= 58 && mx <= 369 && my >= 194 && my <= 244)
            {
                currentLevel = 1;
                loadLevelFromFile(currentLevel);
                gameState = GAME;
            }
            // Level-2 button
            else if (mx >= 427 && mx <= 740 && my >= 194 && my <= 244)
            {
                currentLevel = 2;
                loadLevelFromFile(currentLevel);
                gameState = GAME;
            }
            // Level-3 button
            else if (mx >= 58 && mx <= 369 && my >= 100 && my <= 151)
            {
                currentLevel = 3;
                loadLevelFromFile(currentLevel);
                gameState = GAME;
            }
        }
        else if (gameState == HELP)
        {
            // Back button
            if (mx >= 230 && mx <= 330 && my >= 20 && my <= 55)
            {
                gameState = MENU;
            }
        }

        // For debug (optional)
        printf(" %d %d\n", mx, my);
    }
}

/*
function iMouseWheel() is called when the user scrolls the mouse wheel.
dir = 1 for up, -1 for down.
*/
void iMouseWheel(int dir, int mx, int my)
{
    // place your code here
}

/*
function iKeyboard() is called whenever the user hits a key in keyboard.
key- holds the ASCII value of the key pressed.
*/
void iKeyboard(unsigned char key)
{
    if (gameState == FRONT_PAGE)
    {
        if (key == 13) // Enter
        {
            gameState = MENU;
        }
    }

    if (gameState == MENU)
    {
        /*
        if (key == 's' || key == 'S')
        {
            gameState = GAME;
            gameOver = false;
            gameStartTime = time(NULL);
        }
            */
        if (key == 'e' || key == 'E')
        {
            exit(0);
        }
    }
    else if (gameState == GAME)
    {
        if (key == 'r' || key == 'R')
        {
            // bgX=0;
            gameOver = false;
            pic_x = 320;
            pic_y = 90;
            gameStartTime = time(NULL);
        }
        else if (key == 'p' || key == 'P')
        {
            isPaused = !isPaused; // toggle pause
        }
        else if (key == 'e' || key == 'E')
        {
            exit(0);
        }
        else if (key == 'm' || key == 'M')
        {
            saveGameState();
            gameState = MENU;
        }
    }
}

/*
function iSpecialKeyboard() is called whenver user hits special keys likefunction
keys, home, end, pg up, pg down, arraows etc. you have to use
appropriate constants to detect them. A list is:
GLUT_KEY_F1, GLUT_KEY_F2, GLUT_KEY_F3, GLUT_KEY_F4, GLUT_KEY_F5, GLUT_KEY_F6,
GLUT_KEY_F7, GLUT_KEY_F8, GLUT_KEY_F9, GLUT_KEY_F10, GLUT_KEY_F11,
GLUT_KEY_F12, GLUT_KEY_LEFT, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_DOWN,
GLUT_KEY_PAGE_UP, GLUT_KEY_PAGE_DOWN, GLUT_KEY_HOME, GLUT_KEY_END,
GLUT_KEY_INSERT */

void iAnim()
{
    int idx;

    if (jump)
    {
        iAnimateSprite(&golem);
        return;
    }

    // Predictive horizontal collision check
    Sprite test = golem;

    if (direction == -1) // left
    {
        test.x -= 3;
        idx = collision_idx(&test);
        if (idx == -1)
        {
            golem.x -= 3;
            speed = 0;
        }
        activity(1);
    }
    else if (direction == 1) // right
    {
        test.x += 3;
        idx = collision_idx(&test);
        if (idx == -1)
        {
            if (golem.x > 350)
            {
                speed = -3; // background scroll
            }
            else
            {
                golem.x += 3;
                speed = 0;
            }
        }
        else
        {
            speed = 0;
        }
        activity(1);
    }
    else
    {
        speed = 0;
        activity(0); // idle
    }

    iAnimateSprite(&golem);
}

void animate_tile()
{
    if (direction == 1 && golem.x >= 350)
    {
        for (int i = 0; i < tile_idx; i++)
        {
            tiles[i].x += speed;
        }
    }
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    loadResources();
    // place your own initialization codes here.
    pic_x = 66, pic_y = 122;

    load_bg();
    iSetTimer(100, iAnim);
    iSetTimer(20, animate_tile);
    iSetTimer(20, update_jump);
    iInitialize(800, 500, "Super Mario");
    printf("tile_set[0] width = %d, height = %d\n", tile_set[0].width, tile_set[0].height);

    iInitializeSound();
    bgSoundIdx = iPlaySound("assets/sounds/background.wav", true, 50);
    return 0;
}