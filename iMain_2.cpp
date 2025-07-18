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
// #define MAP_WIDTH 250
#define MAP_WIDTH 200
#define MAP_HEIGHT 17
#define GROUND 122
#define PAUSE_MENU 20
#define LEVEL_COMPLETE 5
#define GAME_OVER_SCREEN 6

int golem_height = 76;
int golem_width = 45;

int pic_x, pic_y;
int golem_idx = 0;
int jump_idx = 0;
int jump = 0;
int jump_speed = 0;
int gravity = 1;
int jump_height = 15;
int ground = GROUND;
int direction = 0;
int tile_x = 0, tile_y = ground;
int tile_width = 30, tile_height = 30;

Image bg[4];
Image golem_idle[5];
Image golem_run_frames[7];
Image golem_jump_frames[11];
Image golem_hurt_frames[3];
Image golem_dead_frames[3];
Image tile_set[3];
Sprite tiles[MAP_HEIGHT * MAP_WIDTH];
Sprite golem;

int speed = 0;
int golemSpeed = 6;
int golem_running = 0;
int animation = -1; 
bool going_right = true;
bool gameOver = false;
bool isPaused = false;
bool touch = false;

int bgSoundIdx = -1;
char tileMap[MAP_HEIGHT][MAP_WIDTH];
char tile_type[MAP_HEIGHT * MAP_WIDTH];
int gameState = FRONT_PAGE;
int gameStartTime = 0;

int bgScrollX = 1;

int tile_idx = 0;
int scroll_x = 0;

bool hasPreviousGame = false;
int prev_pic_x, prev_pic_y;
int prev_jump, prev_jumpSpeed;
bool prev_gameOver;
int prev_gameStartTime;

int currentLevel = 1;
const int maxLevel = 3;

int score = 0;
int frame = 0;

Sprite flag;
int lastBrickX = 0;
bool levelComplete = false;

Image heart_full, heart_empty;
int life = 3; 
bool hurt = false;
int hurtTimer = 0;
bool dead = false;
int deadTimer = 0;
#define MAX_HURT_TIMER 50
#define MAX_DEAD_TIMER 50

char playerName[100] = "";
char highScorer[100] = "None";
int highScore = 0;
bool enteringName = false;
int nameCharIdx = 0;


void loadLevelFromFile(int level);
void activity(int index);

void load_bg()
{
    iLoadImage(&bg[1], "assets/Level1image/BGL1001.png");
    iLoadImage(&bg[2], "Game Project Pic/Level2BG.jpg");
    iLoadImage(&bg[3], "Game Project Pic/Level3BG.jpg");
}
void startLevel(int level)
{
    score = 0;
    life = 3;
    hurt = false;
    hurtTimer = 0;
    dead = false;
    deadTimer = 0;
    tile_idx = 0;
    for (int i = 0; i < MAP_HEIGHT * MAP_WIDTH; i++)
    {
        tile_type[i] = '_';
    }

    scroll_x = 0;
    loadLevelFromFile(level);

    for (int i = 0; i < tile_idx; i++)
    {
        iSetSpritePosition(&tiles[i], tiles[i].x + scroll_x, tiles[i].y);
    }
    iSetSpritePosition(&flag, flag.x + scroll_x, flag.y);

    golem.x = 70;
    golem.y = GROUND;
    direction = 0;
    speed = 0;
    jump = 0;
    jump_speed = 0;
    animation = -1;
    activity(0);

    levelComplete = false;
    gameOver = false;
    gameStartTime = time(NULL);
    gameState = GAME;
    isPaused = false;
}

void loadHighScore()
{
    FILE *fp = fopen("highscore.txt", "r");
    if (fp != NULL)
    {
        fscanf(fp, "%s %d", highScorer, &highScore);
        fclose(fp);
    }
    else
    {
        strcpy(highScorer, "None");
        highScore = 0;
    }
}

void saveHighScore()
{
    FILE *fp = fopen("highscore.txt", "w");
    if (fp != NULL)
    {
        fprintf(fp, "%s %d\n", playerName, score);
        fclose(fp);
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

    char ch;
    for (int i = 0; i < MAP_HEIGHT; i++)
    {
        int j = 0;
        while (j < MAP_WIDTH)
        {
            fscanf(fp, "%c", &ch);
            if (ch == '\n' || ch == '\r')
                continue;
            tileMap[i][j] = ch;
            j++;
        }
    }
    fclose(fp);

    // Load tiles
    iLoadImage(&tile_set[0], "assets/Level1image/Brick_01.png"); // brick
    iLoadImage(&tile_set[1], "assets/Level1image/Coin003.png");  // coin
    iLoadImage(&tile_set[2], "assets/Level1image/Spikes.png");   // spikes

    Image flagImg;
    iLoadImage(&flagImg, "assets/GameBG/Win Flag002.png");

    tile_idx = 0;
    tile_x = 0;
    lastBrickX = 0;
    const int h = tile_height * (MAP_HEIGHT - 1);

    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        tile_y = h - tile_height * y;
        tile_x = 0;
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            char current = tileMap[y][x];

            if (current == '*')
            {
                iInitSprite(&tiles[tile_idx]);
                iChangeSpriteFrames(&tiles[tile_idx], &tile_set[0], 1);
                iSetSpritePosition(&tiles[tile_idx], tile_x, tile_y);
                tile_type[tile_idx] = '*';
                tile_idx++;

                if (tile_x > lastBrickX)
                    lastBrickX = tile_x;

                tile_x += tile_width;
            }
            else if (current == 'o')
            {
                iInitSprite(&tiles[tile_idx]);
                iChangeSpriteFrames(&tiles[tile_idx], &tile_set[1], 1);
                iSetSpritePosition(&tiles[tile_idx], tile_x, tile_y);
                tile_type[tile_idx] = 'o';
                tile_idx++;
                tile_x += tile_width;
            }
            else if (current == 'x')
            {
                iInitSprite(&tiles[tile_idx]);
                iChangeSpriteFrames(&tiles[tile_idx], &tile_set[2], 1);
                iSetSpritePosition(&tiles[tile_idx], tile_x, tile_y);
                tile_type[tile_idx] = 'x';
                tile_idx++;
                tile_x += tile_width;
            }
            else if (current == 'F')
            {
                iInitSprite(&flag);
                iChangeSpriteFrames(&flag, &flagImg, 1);
                iSetSpritePosition(&flag, tile_x, tile_y);
                tile_x += tile_width;
            }

            else if (current == '_')
            {
                tile_x += tile_width;
            }
        }
    }
}

void loadResources()
{
    iLoadFramesFromFolder(golem_idle, "player/idle");
    iLoadFramesFromFolder(golem_run_frames, "Game Project Pic/run");
    iLoadFramesFromFolder(golem_jump_frames, "Game Project Pic/jump");
    iLoadFramesFromFolder(golem_hurt_frames, "player/hurt");
    iLoadFramesFromFolder(golem_dead_frames, "player/dead");
    iInitSprite(&golem);
    iChangeSpriteFrames(&golem, golem_idle, 1);
    iSetSpritePosition(&golem, 70, 122);
    iLoadImage(&heart_full, "assets/Heart/HeartRed.png");    // Use your red heart image
    iLoadImage(&heart_empty, "assets/Heart/HeartBlack.png"); // Use your black heart image
}

void saveGameState()
{
    prev_pic_x = pic_x;
    prev_pic_y = pic_y;
    prev_jump = jump;
    prev_jumpSpeed = jump_speed;
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
    jump_speed = prev_jumpSpeed;
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
        if (current_state == 3)
            iChangeSpriteFrames(&golem, golem_hurt_frames, 3);
        if (current_state == 4)
            iChangeSpriteFrames(&golem, golem_dead_frames, 3);
    }
}

int collision_idx(Sprite *s)
{
    for (int i = 0; i < tile_idx; i++)
    {
        if (tile_type[i] != '*')
            continue;
        if (iCheckCollision(s, &tiles[i]))
        {
            return i;
        }
    }
    return -1;
}

void update_jump()
{
    if (gameState != GAME)
        return;
    Sprite test = golem;
    int idx;

    if (jump)
    {
        test.y = golem.y + jump_speed;
        idx = collision_idx(&test);

        if (idx != -1)
        {

            if (jump_speed < 0)
            {
                golem.y = tiles[idx].y + tile_height;
                jump = 0;
                jump_speed = 0;
                activity(1);
            }
            else
            {
                golem.y = tiles[idx].y - golem_height;
                jump_speed = 0;
            }
        }
        else
        {
            golem.y += jump_speed;
            jump_speed -= gravity;
        }

        if (direction == 1 && golem.x < 350)
        {
            golem.x += golemSpeed;
        }
        else if (direction == 1 && golem.x >= 350)
        {
            speed = -golemSpeed;
        }
        else if (direction == -1)
        {
            golem.x -= golemSpeed;
        }

        activity(2);
    }
    else
    {
        test.y = golem.y - 1;
        idx = collision_idx(&test);

        if (idx == -1)
        {
            jump = 1;
            jump_speed = -1;
            activity(2);
        }
        else
        {
            golem.y = tiles[idx].y + tile_height;
        }
    }

    // coin

    for (int i = 0; i < tile_idx; i++)
    {
        if (tile_type[i] == 'o')
        {
            if (iCheckCollision(&golem, &tiles[i]))
            {
                tile_type[i] = '_';
                // iSetSpritePosition(&tiles[i], -100, -100);
                score += 10;
            }
        }
    }
    // spike
    for (int i = 0; i < tile_idx; i++)
    {
        if (tile_type[i] == 'x')
        {
            if (iCheckCollision(&golem, &tiles[i]) && !hurt)
            {
                life--;
                hurt = true;
                hurtTimer = 0;

                direction = 0;
                speed = 0;
                if (golem.x < tiles[i].x)
                    golem.x = tiles[i].x - tile_width * 3;
                else
                    golem.x = tiles[i].x + tile_width;
                if (life > 0)
                {
                    activity(3); // Hurt animation
                }
                else
                {
                    dead = true;
                    activity(4);
                    frame = 0;
                    deadTimer = 0; // Dead animation
                }
                // hurtTimer = 0;
                // if (life <= 0)
                // {
                //     dead = true;

                //     gameOver = true;
                //     direction = 0;
                // }
            }
        }
    }

    // Flag collision
    if (!levelComplete && iCheckCollision(&golem, &flag))
    {
        levelComplete = true;
        direction = 0;
        speed = 0;
        printf("Level Complete!\n");
        gameState = LEVEL_COMPLETE;
    }
    if (hurt && !dead)
    {
        hurtTimer++;
        if (hurtTimer > MAX_HURT_TIMER) // About 1 second (adjust as needed)
        {
            hurt = false;
            hurtTimer = 0;
        }
    }
    if (dead)
    {
        deadTimer++;
        if (deadTimer > MAX_DEAD_TIMER)
        {
            gameOver = true;
            direction = 0;
            gameState = GAME_OVER_SCREEN;
            if (score > highScore)
{
    enteringName = true;
    nameCharIdx = 0;
    playerName[0] = '\0';
}

        }
    }
}

void iSpecialKeyboard(unsigned char key,int state)
{
    if (gameState == GAME)
    {
        if (key == GLUT_KEY_END)
        {
            printf("%d %d", golem.x, golem.y);
            exit(0);
        }
        if (key == GLUT_KEY_LEFT && !dead)
        {
            direction = -1;
            if (going_right)
            {
                iMirrorSprite(&golem, HORIZONTAL);
                going_right = false;
            }
        }
        if (key == GLUT_KEY_RIGHT && !dead)
        {
            direction = 1;
            if (!going_right)
            {
                iMirrorSprite(&golem, HORIZONTAL);
                going_right = true;
            }
        }
        if (key == GLUT_KEY_UP && !dead)
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

    switch (key)
    {
    case GLUT_KEY_UP:
        iIncreaseVolume(bgSoundIdx, 5);
        break;
    case GLUT_KEY_DOWN:
        iDecreaseVolume(bgSoundIdx, 5);
        break;

    default:
        break;
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
        iShowImage(0, 0, "Game Project Pic/Level BG001.png");
    }

    else if (gameState == GAME)
    {
        iClear();
        iShowLoadedImage(0, 0, &bg[currentLevel]);

        iShowSprite(&golem);

        if (direction == 1 && golem.x >= 350)
        {
            //  bgScrollX
            iWrapImage(&bg[currentLevel], -2);
        }
        char scoreStr[50];
        sprintf(scoreStr, "Score: %d", score);
        iSetColor(255, 255, 255);
        iText(20, 460, scoreStr, GLUT_BITMAP_HELVETICA_18);

        iSetColor(100, 100, 100);
        iFilledRectangle(740, 450, 50, 30);
        iSetColor(255, 255, 255);
        iText(750, 460, "Pause", GLUT_BITMAP_HELVETICA_12);

        if (gameOver)
        {
            iSetColor(255, 0, 0);
            iText(300, 250, "Game Over! Press R to Restart", GLUT_BITMAP_HELVETICA_18);
        }

        for (int i = 0; i < tile_idx; i++)
        {
            if (tile_type[i] != '_')
            {
                iShowSprite(&tiles[i]);
            }
        }
        for (int i = 0; i < 3; i++)
        {
            if (i < life)
                iShowLoadedImage(20 + i * 35, 400, &heart_full); // red heart
            else
                iShowLoadedImage(20 + i * 35, 400, &heart_empty); // black heart
        }

        // Show the flag sprite
        iShowSprite(&flag);
    }
    else if (gameState == PAUSE_MENU)
    {
        iClear();
        iShowImage(0, 0, "assets/GameBG/Pause002.png");
    }
    else if (gameState == GAME_OVER_SCREEN)
    {
        iClear();
        iShowImage(0, 0, "assets/GameBG/Gameoverbg001.png");
        iSetColor(255, 255, 255);
    char msg[200];
    sprintf(msg, "Your Score: %d", score);
    iText(300, 350, msg, GLUT_BITMAP_TIMES_ROMAN_24);

    sprintf(msg, "High Score: %d by %s", highScore, highScorer);
    iText(250, 310, msg, GLUT_BITMAP_TIMES_ROMAN_24);

    if (enteringName)
    {
        iSetColor(255, 255, 0);
        iText(220, 260, "New High Score! Enter your name:", GLUT_BITMAP_TIMES_ROMAN_24);
        iText(300, 230, playerName, GLUT_BITMAP_HELVETICA_18);
    }

    }

    else if (gameState == LEVEL_COMPLETE)
    {
        iClear();
        iShowImage(0, 0, "assets/GameBG/Level Complete001.png");
    }
}
void resetTilePositionsForNewGame()
{
    for (int i = 0; i < tile_idx; i++)
    {
        iSetSpritePosition(&tiles[i], tiles[i].x + scroll_x, tiles[i].y);
    }
    iSetSpritePosition(&flag, flag.x + scroll_x, flag.y);
    scroll_x = 0;
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
            // New Game
            if (mx >= 208 && mx <= 596 && my >= 330 && my <= 385)
            {
                gameState = LEVEL_SELECT;
            }
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
            // Previous Game
            else if (mx >= 208 && mx <= 596 && my >= 270 && my <= 320)
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
            if (mx >= 58 && mx <= 369 && my >= 194 && my <= 244)
            {
                currentLevel = 1;
                startLevel(currentLevel);
            }
            else if (mx >= 427 && mx <= 740 && my >= 194 && my <= 244)
            {
                currentLevel = 2;
                startLevel(currentLevel);
            }
            else if (mx >= 58 && mx <= 369 && my >= 100 && my <= 151)
            {
                currentLevel = 3;
                startLevel(currentLevel);
            }
        }
        else if (gameState == HELP)
        {
            if (mx >= 230 && mx <= 330 && my >= 20 && my <= 55)
            {
                gameState = MENU;
            }
        }
        else if (gameState == GAME)
        {
            if (mx >= 740 && mx <= 790 && my >= 450 && my <= 480)
            {
                gameState = PAUSE_MENU;
                isPaused = true;
            }
        }
        else if (gameState == PAUSE_MENU)
        {
            if (mx >= 278 && mx <= 518 && my >= 300 && my <= 350)
            {
                gameState = GAME;
                isPaused = false;
            }
            else if (mx >= 278 && mx <= 518 && my >= 230 && my <= 280)
            {
                currentLevel = 1;
                startLevel(currentLevel);
            }
            else if (mx >= 278 && mx <= 518 && my >= 160 && my <= 210)
            {
                printf("Settings clicked\n");
            }
            else if (mx >= 278 && mx <= 518 && my >= 90 && my <= 140)
            {
                gameState = MENU;
                isPaused = false;
            }
        }
        else if (gameState == LEVEL_COMPLETE)
        {
            if (mx >= 127 && mx <= 356 && my >= 68 && my <= 138)
            {
                if (currentLevel < maxLevel)
                {
                    currentLevel++;
                    startLevel(currentLevel);
                }
            }

            // Main Menu
            else if (mx >= 418 && mx <= 608 && my >= 68 && my <= 138)
            {
                gameState = MENU;
            }
        }
         else if (gameState == GAME_OVER_SCREEN)
         {
           //   Retry Button
             if (mx >= 185 && mx <= 380 && my >= 53 && my <= 115)
             {
                 life = 3;
                 startLevel(currentLevel);
                 gameState = GAME;
             }

            // Menu Button
            else if (mx >= 415 && mx <= 608 && my >= 53 && my <= 115)
            {
                gameState = MENU;
            }
        }



        

        // Debug mouse positio
        // For debug (optional)
        printf(" %d %d\n", mx, my);
    }
}

/*
function iMouseWheel() is called when the user scrolls the mouse wheel.
dir = 1 for up for down.
*/
void iMouseWheel(int dir, int mx, int my)
{
    // place your code here
}

/*
function iKeyboard() is called whenever the user hits a key in keyboard.
key- holds the ASCII value of the key pressed.
*/
void iKeyboard(unsigned char key, int state)
{
    // --------- NAME ENTRY MODE ---------
    if (enteringName)
    {
        // Process input only on key press (GLUT_DOWN)
        if (state == GLUT_DOWN)
        {
            if (key == '\b') // Backspace
            {
                if (nameCharIdx > 0)
                {
                    nameCharIdx--;
                    playerName[nameCharIdx] = '\0';
                }
            }
            else if (key == '\r' || key == 13) // Enter key
            {
                enteringName = false;
                saveHighScore();
                gameState = GAME_OVER_SCREEN;
            }
            else if (key >= 32 && key <= 126 && nameCharIdx < 20)
            {
                playerName[nameCharIdx++] = key;
                playerName[nameCharIdx] = '\0';
            }
        }
        return; // ✅ Prevent further processing
    }

    // --------- FRONT PAGE ---------
    if (gameState == FRONT_PAGE)
    {
        if (key == 13 && state == GLUT_DOWN) // Enter key
        {
            gameState = MENU;
        }
    }

    // --------- MENU ---------
    else if (gameState == MENU)
    {
        if ((key == 'e' || key == 'E') && state == GLUT_DOWN)
        {
            exit(0);
        }
    }

    // --------- GAME STATE ---------
    else if (gameState == GAME)
    {
        if ((key == 'r' || key == 'R') && state == GLUT_DOWN)
        {
            gameOver = false;
            life = 3;
            startLevel(currentLevel);
        }
        else if ((key == 'p' || key == 'P') && state == GLUT_DOWN)
        {
            isPaused = !isPaused;
        }
        else if ((key == 'e' || key == 'E') && state == GLUT_DOWN)
        {
            exit(0);
        }
        else if ((key == 'm' || key == 'M') && state == GLUT_DOWN)
        {
            saveGameState();
            gameState = MENU;
        }
    }

    // --------- SOUND CONTROLS ---------
    if (state == GLUT_DOWN)
    {
        if (key == 'r') iResumeSound(bgSoundIdx);
        else if (key == 'p') iPauseSound(bgSoundIdx);
        else if (key == 'x') iStopSound(bgSoundIdx);
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

    if (gameState != GAME)
        return;
    int idx;

    if (dead)
    {
        // iAnimateSprite(&golem);
        if (frame < 2)
        {
            iAnimateSprite(&golem);
            frame++;
        }
        else
        {
            golem.currentFrame = 2;
        }
        return;
    }

    if (hurt)
    {
        iAnimateSprite(&golem);
        if (hurtTimer > MAX_HURT_TIMER)
            return;
    }

    if (jump)
    {
        iAnimateSprite(&golem);
        return;
    }

    // Predictive horizontal collision check
    Sprite test = golem;

    if (direction == -1) // left
    {
        idx = collision_idx(&test);
        if (idx == -1)
        {
            golem.x = golem.x - golemSpeed;
            speed = 0;
        }
        activity(1);
    }
    else if (direction == 1) // right
    {
        test.x = golem.x + golemSpeed;
        idx = collision_idx(&test);
        if (idx == -1)
        {
            // Prevent golem from going beyond last brick + flag
            if (golem.x + scroll_x >= lastBrickX + tile_width + 150)
            {
                direction = 0;
                return;
            }

            if (golem.x > 350)
            {
                speed = -golemSpeed; // background scroll
            }
            else
            {
                golem.x += golemSpeed;
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

    if (gameState != GAME)
        return;
    if (direction == 1 && golem.x >= 350)
    {
        // Stop scrolling when last brick is visible on screen
        if (lastBrickX + tile_width - scroll_x <= 800)

        {
            speed = 0;
            return;
        }

        scroll_x += golemSpeed;

        for (int i = 0; i < tile_idx; i++)
        {
            tiles[i].x -= golemSpeed;
        }

        flag.x -= golemSpeed;

        // bgScrollX += golemSpeed / 4;
    }
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    loadResources();
    loadHighScore();
    pic_x = 66, pic_y = 122;

    load_bg();
    iSetTimer(100, iAnim);
    iSetTimer(19, animate_tile);
    iSetTimer(30, update_jump);

    iInitializeSound();
    bgSoundIdx = iPlaySound("assets/sounds/BGSound001.wav", true, 50);
    // gameOverSoundIdx = iPlaySound("assets/sounds/Game over sound.mp3",true,30);

    iOpenWindow(800, 500, "Super Mario");
    //  printf("tile_set[0] width = %d, height = %d\n", tile_set[0].width, tile_set[0].height);

    return 0;
}