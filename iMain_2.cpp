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
#define MAP_WIDTH 250
#define MAP_HEIGHT 17
#define GROUND 122
#define PAUSE_MENU 20
#define LEVEL_COMPLETE 5


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
Image golem_idle[2];
Image golem_run_frames[7];
Image golem_jump_frames[11];
Image tile_set[2];
Sprite tiles[MAP_HEIGHT * MAP_WIDTH];
Sprite golem;

int speed = 0;
int golemSpeed = 3;
int golem_running = 0;
int animation = -1; // 0 idle, 1 run, 2 jump
bool going_right = true;
bool gameOver = false;
bool isPaused = false;
bool touch = false;

int bgSoundIdx = -1;
char tileMap[MAP_HEIGHT][MAP_WIDTH];
char tile_type[MAP_HEIGHT * MAP_WIDTH];
int gameState = FRONT_PAGE;
int gameStartTime = 0;

const int bgScrollSpeed = 2;
int bgScrollX = 0;

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

Sprite flag;
int lastBrickX = 0;
bool levelComplete = false;

<<<<<<< HEAD



=======
>>>>>>> cceb638ba58dca812a6482f9e59c29dc115940c6
void load_bg()
{
    // Load backgrounds for 3 levels   Level1BG.png
    iLoadImage(&bg[1], "assets/Level1image/BGL1001.png");
    iLoadImage(&bg[2], "Game Project Pic/Level2BG.jpg");
    iLoadImage(&bg[3], "Game Project Pic/Level3BG.jpg");
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
                continue; // skip line breaks
            tileMap[i][j] = ch;
            j++;
        }
    }
    fclose(fp);

    // Load tiles
    iLoadImage(&tile_set[0], "assets/Level1image/Brick_01.png"); // brick
    iLoadImage(&tile_set[1], "assets/Level1image/Coin003.png");  // coin

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
    iLoadFramesFromFolder(golem_idle, "Game Project Pic/walk");
    iLoadFramesFromFolder(golem_run_frames, "Game Project Pic/run");
    iLoadFramesFromFolder(golem_jump_frames, "Game Project Pic/jump");
    iInitSprite(&golem);
    iChangeSpriteFrames(&golem, golem_idle, 1);
    iSetSpritePosition(&golem, 70, 122);
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
    }
}

int collision_idx(Sprite *s)
{
    for (int i = 0; i < tile_idx; i++)
    {
        if(tile_type[i] != '*') continue;
        if (iCheckCollision(s, &tiles[i]))
        {
            return i;
        }
    }
    return -1;
}

// void update_jump()
// {
//     Sprite test = golem;
//     int idx;

//     if (jump)
//     {

//         test.y = golem.y + jump_speed;

//         idx = collision_idx(&test);

//         if (idx != -1)
//         {

//             if (jump_speed < 0)
//             {
//                 golem.y = tiles[idx].y + tile_height;
//                 jump = 0;
//                 jump_speed = 0;
//                 activity(1);
//             }
//             else
//             {
//                 golem.y = tiles[idx].y - golem_height;
//                 jump_speed = 0;
//             }
//         }
//         else
//         {

//             golem.y += jump_speed;
//             jump_speed -= gravity;
//         }

//         if (direction == 1 && golem.x < 350)
//         {
//             golem.x += golemSpeed;
//             activity(2);
//         }
//         else if (direction == 1 && golem.x >= 350)
//         {
//             speed = -golemSpeed;
//         }
//         else if (direction == -1)
//         {
//             golem.x -= golemSpeed;
//             activity(2);
//         }
//     }
//     else
//     {
//         test.y = golem.y - 1;
//         idx = collision_idx(&test);
//         if (idx == -1)
//         {
//             jump = 1;
//             jump_speed = -1;
//         }
//     }
//     for (int i = 0; i < tile_idx; i++)
//     {
//         if (tile_type[i] == 'o')
//         {
//             Sprite coin_test = tiles[i];
//             if (iCheckCollision(&golem, &coin_test))
//             {
//                 tile_type[i] = '_';
//                 // iSetSpritePosition(&tiles[i], -100, -100);
//                 score += 10;
//             }
//         }
//     }
//     if (!levelComplete && iCheckCollision(&golem, &flag))
//     {
//         levelComplete = true;
//         printf("Level Complete!\n");
//         gameState = LEVEL_SELECT;
//     }
// }


void update_jump()
{
    Sprite test = golem;
    int idx;

    if (jump) 
    {
<<<<<<< HEAD

        test.y = golem.y + jump_speed;


=======
        test.y = golem.y + jump_speed;

>>>>>>> cceb638ba58dca812a6482f9e59c29dc115940c6
        idx = collision_idx(&test);

        if (idx != -1)
        {
<<<<<<< HEAD

            if (jump_speed < 0)
            {
                // Landing on top of tile
                golem.y = tiles[idx].y + tile_height;
                jump = 0;
                jump_speed = 0;
                activity(1);
            }
            else
            {
                // Hitting head on tile from below (ceiling)
                golem.y = tiles[idx].y - golem_height;
                jump_speed = 0; // Stop upward movement
            }

        }
        else
        {

=======
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
>>>>>>> cceb638ba58dca812a6482f9e59c29dc115940c6
            golem.y += jump_speed;
            jump_speed -= gravity; 
        }
<<<<<<< HEAD


=======
>>>>>>> cceb638ba58dca812a6482f9e59c29dc115940c6
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
        }
    }
<<<<<<< HEAD
    // Check for coin collection (always check, regardless of jump)
    for (int i = 0; i < tile_idx; i++)
    {
        if (tile_type[i] == 'o') // only if tile is coin
        {
            Sprite coin_test = tiles[i];
            if (iCheckCollision(&golem, &coin_test))
            {
                tile_type[i] = '_';               // mark as collected
                iSetSpritePosition(&tiles[i], -100, -100); // move out of view
                score += 10;                      // increase score
                // Optional: play coin sound
            }
        }
    }

// Check flag collision
    if (!levelComplete && iCheckCollision(&golem, &flag))
    {
        levelComplete = true;
        printf("Level Complete!\n");

        // Optional: go to menu or next level
        gameState = LEVEL_COMPLETE;
    }




=======

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
>>>>>>> cceb638ba58dca812a6482f9e59c29dc115940c6

    if (!levelComplete && iCheckCollision(&golem, &flag))
    {
        levelComplete = true;
        printf("Level Complete!\n");
        gameState = LEVEL_SELECT;
    }
}


void iSpecialKeyboard(unsigned char key, int state)
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
            iWrapImage(&bg[currentLevel], bgScrollX);
        }
        else
        {
            iWrapImage(&bg[currentLevel], 0);
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

        // ✅ Show the flag sprite
        iShowSprite(&flag);
    }
    else if (gameState == PAUSE_MENU)
    {
        iClear();
        iShowImage(0, 0, "assets/GameBG/Pause002.png");
    }
    else if (gameState == LEVEL_COMPLETE)
{
    iClear();
   // iShowImage(0, 0, "assets/GameBG/LevelComplete001.png");

    iSetColor(255, 255, 255);
    iText(300, 300, "Level Complete!", GLUT_BITMAP_TIMES_ROMAN_24);

    // Draw buttons
    iSetColor(100, 100, 100);
    iFilledRectangle(250, 180, 300, 50); // Next Level
    iFilledRectangle(250, 100, 300, 50); // Menu

    iSetColor(255, 255, 255);
    iText(340, 200, "Next Level");
    iText(360, 120, "Main Menu");
}

}
void resetTilePositionsForNewGame()
{
    for (int i = 0; i < tile_idx; i++)
    {
        iSetSpritePosition(&tiles[i], tiles[i].x + scroll_x, tiles[i].y);
    }
    iSetSpritePosition(&flag, flag.x + scroll_x, flag.y); // ✅ adjust flag position too
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
                golem.x = 70;
                golem.y = GROUND;
                direction = 0;
                speed = 0;
                jump = 0;
                jump_speed = 0;
                animation = -1;
                activity(0);

<<<<<<< HEAD

=======
>>>>>>> cceb638ba58dca812a6482f9e59c29dc115940c6
                // Reset scroll BEFORE changing tile positions
                for (int i = 0; i < tile_idx; i++)
                {
                    iSetSpritePosition(&tiles[i], tiles[i].x + scroll_x, tiles[i].y);
                }
                scroll_x = 0;
<<<<<<< HEAD

=======
>>>>>>> cceb638ba58dca812a6482f9e59c29dc115940c6

                gameOver = false;
                gameStartTime = time(NULL);
                gameState = GAME;
                isPaused = false;
            }
            // Level-2 button
            else if (mx >= 427 && mx <= 740 && my >= 194 && my <= 244)
            {
                currentLevel = 2;
                loadLevelFromFile(currentLevel);
                golem.x = 70;
                golem.y = GROUND;
                direction = 0;
                speed = 0;
                jump = 0;
                jump_speed = 0;
                animation = -1;
                activity(0);

                // Reset camera
                // Reset scroll BEFORE changing tile positions
                for (int i = 0; i < tile_idx; i++)
                {
                    iSetSpritePosition(&tiles[i], tiles[i].x + scroll_x, tiles[i].y);
                }
                scroll_x = 0;
<<<<<<< HEAD


=======
>>>>>>> cceb638ba58dca812a6482f9e59c29dc115940c6

                gameOver = false;
                gameStartTime = time(NULL);
                gameState = GAME;
                isPaused = false;
            }
            // Level-3 button
            else if (mx >= 58 && mx <= 369 && my >= 100 && my <= 151)
            {
                currentLevel = 3;
                loadLevelFromFile(currentLevel);
                golem.x = 70;
                golem.y = GROUND;
                direction = 0;
                speed = 0;
                jump = 0;
                jump_speed = 0;
                animation = -1;
                activity(0);

                // Reset camera
                // Reset scroll BEFORE changing tile positions
                for (int i = 0; i < tile_idx; i++)
                {
                    iSetSpritePosition(&tiles[i], tiles[i].x + scroll_x, tiles[i].y);
                }
                scroll_x = 0;
<<<<<<< HEAD

=======
>>>>>>> cceb638ba58dca812a6482f9e59c29dc115940c6

                gameOver = false;
                gameStartTime = time(NULL);
                gameState = GAME;
                isPaused = false;
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
        else if (gameState == GAME)
        {
            // If pause button clicked
            if (mx >= 740 && mx <= 790 && my >= 450 && my <= 480)
            {
                gameState = PAUSE_MENU;
                isPaused = true;
            }
        }
        else if (gameState == PAUSE_MENU)
        {
            // Resume
            if (mx >= 278 && mx <= 518 && my >= 300 && my <= 350)
            {
                gameState = GAME;
                isPaused = false;
            }
            // New Game
            else if (mx >= 278 && mx <= 518 && my >= 230 && my <= 280)
            {
                // Reset everything for new game
                currentLevel = 1;
                loadLevelFromFile(currentLevel);

                golem.x = 70;
                golem.y = GROUND;
                direction = 0;
                speed = 0;
                jump = 0;
                jump_speed = 0;
                animation = -1;
                activity(0);

                // Reset camera
                // Reset scroll BEFORE changing tile positions
                for (int i = 0; i < tile_idx; i++)
                {
                    iSetSpritePosition(&tiles[i], tiles[i].x + scroll_x, tiles[i].y);
                }
                scroll_x = 0;
<<<<<<< HEAD


=======
>>>>>>> cceb638ba58dca812a6482f9e59c29dc115940c6

                gameOver = false;
                gameStartTime = time(NULL);
                gameState = GAME;
                isPaused = false;
            }
            // Settings (optional)
            else if (mx >= 278 && mx <= 518 && my >= 160 && my <= 210)
            {
                // Just a placeholder for now
                printf("Settings clicked\n");
            }
            // Main Menu
            else if (mx >= 278 && mx <= 518 && my >= 90 && my <= 140)
            {
                gameState = MENU;
                isPaused = false;
            }
            


        }
        else if (gameState == LEVEL_COMPLETE)
{
    // Next Level
    if (mx >= 250 && mx <= 550 && my >= 180 && my <= 230)
    {
        if (currentLevel < maxLevel)
        {
            currentLevel++;
            loadLevelFromFile(currentLevel);
            golem.x = 70;
            golem.y = GROUND;
            direction = 0;
            speed = 0;
            jump = 0;
            jump_speed = 0;
            animation = -1;
            activity(0);

            scroll_x = 0;
            for (int i = 0; i < tile_idx; i++)
            {
                iSetSpritePosition(&tiles[i], tiles[i].x + scroll_x, tiles[i].y);
            }

            levelComplete = false;
            gameOver = false;
            gameStartTime = time(NULL);
            gameState = GAME;
        }
    }
    // Main Menu
    else if (mx >= 250 && mx <= 550 && my >= 100 && my <= 150)
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
            gameOver = false;
            pic_x = 320;
            pic_y = 90;
            gameStartTime = time(NULL);
        }
        else if (key == 'p' || key == 'P')
        {
            isPaused = !isPaused;
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

    switch (key)
    {
    case 'r':
        iResumeSound(bgSoundIdx);
        break;
    case 'p':
        iPauseSound(bgSoundIdx);
        break;
    case 'x':
        iStopSound(bgSoundIdx);
        break;
    // place your codes for other keys here
    default:
        break;
    }
    if (key == '+') // faster
    {
        if (golemSpeed < 15)
            golemSpeed++;
    }
    else if (key == '-') // slower
    {
        if (golemSpeed > 1)
            golemSpeed--;
    }
<<<<<<< HEAD

}

=======
}
>>>>>>> cceb638ba58dca812a6482f9e59c29dc115940c6

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
<<<<<<< HEAD
            if (golem.x + scroll_x >= lastBrickX + tile_width + 150)
=======
            if (golem.x + scroll_x >= lastBrickX + tile_width + 50)
>>>>>>> cceb638ba58dca812a6482f9e59c29dc115940c6
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
/*
void animate_tile()
{
    if (direction == 1 && golem.x >= 350)
    {
        for (int i = 0; i < tile_idx; i++)
        {
            tiles[i].x += speed;
        }
         bgScrollX += abs(speed) / 4;
    }

}
    */

void animate_tile()
{
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

        bgScrollX += golemSpeed / 4;
    }
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    loadResources();
    pic_x = 66, pic_y = 122;

    load_bg();
    iSetTimer(100, iAnim);
    iSetTimer(20, animate_tile);
    iSetTimer(20, update_jump);

    iInitializeSound();
    bgSoundIdx = iPlaySound("assets/sounds/background.wav", true, 50);
    iOpenWindow(800, 500, "Super Mario");
    //  printf("tile_set[0] width = %d, height = %d\n", tile_set[0].width, tile_set[0].height);

    return 0;
}