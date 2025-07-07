#include "iGraphics.h"
#include <iostream>
#include "iSound.h"
#include <windows.h>
using namespace std;

int pic_x=320, pic_y=90;
int idle_idx = 0;
char run_idle[18][100];
// int groundY = 100;           // Ground level
int jump = 0;                   // Jump flag
int jumpSpeed = 0;              // Current vertical speed
int gravity = 1;                // Gravity strength
int jumpHeight = 14;
bool rightPressed = false;
bool leftPressed = false;
int moveSpeed = 10;
bool gameOver = false;
bool isPaused = false;

#define MENU 0
#define GAME 1
#define GAME_OVER 2
#define FRONT_PAGE -1
#define HELP 3
#define LEVEL_SELECT  4      // <-- new
#define GAME_WIN 5
#define LEVEL_COMPLETE 10
#define PAUSE_MENU 20




int gameState = FRONT_PAGE;
int gameStartTime = 0;
int levelCompleteTime = 0;  // time when level completed


// int bgX = 0;
const int bgScrollSpeed = 2;

Image bg[4]; // We'll use bg[1], bg[2], and bg[3] for Level 1–3

int speed= 1;

int bgSoundIdx = -1;

bool hasPreviousGame = false;
int prev_pic_x, prev_pic_y;
int prev_jump, prev_jumpSpeed;
// int prev_bgX;
bool prev_gameOver;
int prev_gameStartTime;

int  currentLevel = 1;
const int maxLevel = 3;


#define MAP_WIDTH 100
#define MAP_HEIGHT 16

int tileMap[MAP_HEIGHT][MAP_WIDTH];

static int keyHoldCounter = 0;

int cameraX = 0; // Scroll offset in pixels

int coinCount = 0;  
int score = 0;       

bool reachedWinFlag = false;
#define WIN_FLAG_TILE 99




void loadLevelFromFile(int level)
{
    char filename[100];
    sprintf(filename, "assets/Level/level%d.txt", level);  

    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Failed to open %s\n", filename);
        return;
    }

    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            fscanf(fp, "%d", &tileMap[y][x]);
        }
    }

    fclose(fp);
}

/*

Image bg;
Image golem_idle[2];
Image golem_run_frames[7];
Image golem_jump_frames[11];
Sprite golem;

int tile_x=300,tile_y=122;
int speed=0;
int golem_running=0;
int animation=-1;// 0 idle, 1 run, 2 jump
int direction=0;

*/
void populate_run_images()
{

    for (int i = 0; i < 8; i++)
    {
        sprintf(run_idle[i], "assets/Run/Run%03d.png", i);
    }
}

void update_run()
{
    idle_idx = (idle_idx + 1) % 8;
    // 0 1 2 3 0 1 2 3
}

/*
function iDraw() is called again and again by the system.
*/


void saveGameState()
{
    prev_pic_x = pic_x;
    prev_pic_y = pic_y;
    prev_jump = jump;
    prev_jumpSpeed = jumpSpeed;
    prev_gameOver = gameOver;
    prev_gameStartTime = gameStartTime;
    hasPreviousGame = true;
}
void loadGameState()
{
    if (!hasPreviousGame) return;

    pic_x = prev_pic_x;
    pic_y = prev_pic_y;
    jump = prev_jump;
    jumpSpeed = prev_jumpSpeed;
    gameOver = prev_gameOver;
    gameStartTime = prev_gameStartTime;
}
void loadResources()
{
    // Load backgrounds for 3 levels   Level1BG.png
    iLoadImage(&bg[1], "assets/GameBG/BGL1001.png");
    iLoadImage(&bg[2], "assets/GameBG/Level2BG.jpg");
    iLoadImage(&bg[3], "assets/GameBG/Level3BG.jpg");

    // Resize all to fit screen
    for (int i = 1; i <= 3; i++)
    {
        iResizeImage(&bg[i], 800, 500);
    }
}

void iDraw()
{
    iClear();

    if (gameState == FRONT_PAGE)
    {
        iShowImage(0, 0, "assets/GameBG/1st Cover002.png");
        iSetColor(0, 0, 0);
        iText(10, 10, "Press Enter to Continue or Click Main Menu", GLUT_BITMAP_HELVETICA_18);
        return;
    }
    else if(gameState == MENU)
    {

        iShowImage(0, 0,"assets/GameBG/2nd cover003.png");

        iSetColor(0, 0, 0);
        iText(10, 10, "Press e to Exit or Click Exit button.", GLUT_BITMAP_HELVETICA_18);
        iSetColor(0, 0, 0);
        iText(10, 40, "Press New game to start the game.", GLUT_BITMAP_HELVETICA_18);


        return;

    }
    else if (gameState == HELP)
    {
        iClear();
        iShowImage(0, 0,"assets/GameBG/Help Cover001.png");

        iSetColor(0, 0, 0);
        iText(300, 450, "HELP", GLUT_BITMAP_TIMES_ROMAN_24);

        iText(100, 310, "1. Use RIGHT and LEFT arrow keys to move");
        iText(100, 280, "2. Press SPACE to jump");
        iText(100, 250, "3. Avoid obstacles to survive");
        iText(100, 220, "4. Press 'R' to restart the game after Game Over");
        iText(100, 190, "5. Press 'E' to exit the game");
        // Back Button
        iSetColor(100, 100, 100);
        iFilledRectangle(230, 20, 100, 35);
        iSetColor(255, 255, 255);
        iText(265, 35, "Back");
    }

    else if (gameState == LEVEL_SELECT)
    {
        iClear();
        iShowImage(0, 0, "assets/GameBG/Level BG001.png");   // optional background


    }



    else if (gameState == GAME)
    {


        iClear();
        iShowLoadedImage(0, 0, &bg[currentLevel]);
        iText(70, 30, "Press P to pause and resume.", GLUT_BITMAP_TIMES_ROMAN_24);
        iText(70, 10, "Press M to go to Menu.", GLUT_BITMAP_TIMES_ROMAN_24);

        // Draw pause button (top-right corner)
iSetColor(100, 100, 100);
iFilledRectangle(740, 450, 50, 30);
iSetColor(255, 255, 255);
iText(750, 460, "Pause", GLUT_BITMAP_HELVETICA_12);


        if (gameOver)
        {
            iSetColor(255, 0, 0);
            iText(300, 250, "Game Over! Press R to Restart", GLUT_BITMAP_HELVETICA_18);
        }

        // This loop draws the level based on the tileMap
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            for (int x = 0; x < MAP_WIDTH; x++)
            {
                int tile = tileMap[y][x];

                int drawX = x * 30 - cameraX;  // Subtract cameraX for side-scrolling

                int drawY = ((MAP_HEIGHT - 1 - y) * 30); // 30 is the height of a tile

                if (tile == 1) // Coin Block
                {

                    if (pic_x + 30 > drawX && pic_x < drawX + 30 && pic_y + 30 > drawY && pic_y < drawY + 30)
                    {
                        coinCount++;    // Increase coin count
                        score += 10;    // Add score for coin
                        tileMap[y][x] = 0; // Remove coin
                    }
                    else
                    {
                        iShowImage(drawX, drawY, "assets/Level1image/Coin003.png");
                    }

                }
                else if (tile == 2) // Brown Brick
                {
                    iShowImage(drawX, drawY, "assets/Level1image/Brick_01.png");
                }
                else if (tile == 3) // Green/Stone Brick
                {
                    iShowImage(drawX, drawY, "assets/Level1image/Brick_01.png");
                }
                else if (tile == 4) // Ground 1 Block
                {
                    iShowImage(drawX, drawY, "assets/Level1image/Ground_01.png");
                }
                else if (tile == 5) // Ground 2 block Decorative Ground
                {
                    iShowImage(drawX, drawY, "assets/Level1image/Ground_02.png");
                }
                else if (tile == 6) // Ground 3 block Wooden Crate
                {
                    iShowImage(drawX, drawY, "assets/Level1image/Ground_03.png");
                }
                else if (tile == 7) // Ground up 1 block Wooden Barrel
                {
                    iShowImage(drawX, drawY, "assets/Level1image/Ground_04.png");
                }
                else if (tile == 8) // Ground up 2 block Ladder
                {
                    iShowImage(drawX, drawY, "assets/Level1image/Ground_05.png");
                }
                else if (tile == 9) // Ground up 3 block Spikes
                {
                    iShowImage(drawX, drawY, "assets/Level1image/Ground_07.png");
                }
                else if (tile == 10) // Ground up 4 block Bridge
                {
                    iShowImage(drawX, drawY, "assets/Level1image/Ground_08.png");
                }
                else if (tile == WIN_FLAG_TILE) // Win flag tile
                {
                    iShowImage(drawX, drawY, "assets/GameBG/Win Flag002.png");

                    // Check collision with Mario
                    if (pic_x + 30 > drawX && pic_x < drawX + 30 &&
                            pic_y + 30 > drawY && pic_y < drawY + 30)
                    {
                        reachedWinFlag = true;
                    }
                }
            }
        }

        iShowImage(pic_x, pic_y,  run_idle[idle_idx]);


        char scoreText[100];
        sprintf(scoreText, "Score: %d  Coins: %d", score, coinCount);
        iSetColor(255, 255, 255);
        iText(10, 480, scoreText, GLUT_BITMAP_HELVETICA_18);
    }
    else if (gameState == GAME_WIN)
    {
        iClear();
        // iShowImage(0, 0, "assets/Level1image/GameWinBG.jpg"); // <-- use your own image path

        iSetColor(255, 215, 0);
        iText(280, 300, "Congratulations!", GLUT_BITMAP_TIMES_ROMAN_24);
        iText(240, 260, "You completed all levels!", GLUT_BITMAP_HELVETICA_18);
        iText(230, 180, "Press 'M' to return to Main Menu.", GLUT_BITMAP_HELVETICA_18);
        iText(230, 150, "Press 'E' to exit.", GLUT_BITMAP_HELVETICA_18);
    }
    else if (gameState == PAUSE_MENU)
{
    iClear();
   iShowImage(0,0,"assets/GameBG/Pause001.png");
}

    else if (gameState == LEVEL_COMPLETE)
{
    iClear();
    iSetColor(0, 255, 0);
    char levelText[100];
    sprintf(levelText, "Level %d Complete!", currentLevel);
    iText(300, 300, levelText, GLUT_BITMAP_TIMES_ROMAN_24);

    iSetColor(100, 100, 100);
    iFilledRectangle(280, 200, 200, 50);
    iSetColor(255, 255, 255);
    iText(330, 220, "Next Level", GLUT_BITMAP_HELVETICA_18);

    iSetColor(150, 150, 150);
    iFilledRectangle(280, 130, 200, 50);
    iSetColor(255, 255, 255);
    iText(320, 150, "Back to Menu", GLUT_BITMAP_HELVETICA_18);
}

}

void iUpdate()
{
    if (gameOver || isPaused)
        return;

    // Horizontal movement while in air or ground
    /*
    if (rightPressed)
    {
        if (pic_x < 400)
            pic_x += moveSpeed;
        else
            cameraX += moveSpeed;
    }
            */
    int flagVisible = 0;

// Step 1: Search for visible flag on screen
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            if (tileMap[y][x] == WIN_FLAG_TILE)
            {
                int flagX = x * 30;
                if (flagX - cameraX >= 0 && flagX - cameraX <= 800)
                {
                    flagVisible = 1;
                    break;
                }
            }
        }
  if (flagVisible) break;
    }

// Step 2: Move logic
    if (rightPressed)
    {
        if (!flagVisible)
        {
            cameraX += moveSpeed;
        }
        else
        {
            if (pic_x + 30 < 800)  // Let Mario move manually to right side
                pic_x += moveSpeed;
        }
    }


    if (leftPressed)
    {
        if (cameraX > 0)
            cameraX -= moveSpeed;
        else if (pic_x > 100)
            pic_x -= moveSpeed;
    }
  if (rightPressed)
    {
        cameraX += moveSpeed;
    }

  if (leftPressed)
    {
        if (cameraX > 0)
            cameraX -= moveSpeed;
    }



    // Jump logic
    if (jump)
    {
        pic_y += jumpSpeed;
        jumpSpeed -= gravity;

        if (pic_y <= 90)
        {
            pic_y = 90;
            jump = 0;
            jumpSpeed = 0;
        }
    }
    if (rightPressed || leftPressed)
    {
        keyHoldCounter++;
        if (keyHoldCounter > 5)    // ~100ms (if iUpdate is 20ms)
        {
            rightPressed = false;
            leftPressed = false;
            keyHoldCounter = 0;
        }
    }
   if (reachedWinFlag)
   {
    reachedWinFlag = false;
    gameState = LEVEL_COMPLETE;
    levelCompleteTime = time(NULL);
}



    int maxCameraX = MAP_WIDTH * 30 - 800; // total map width in pixels - screen width
    if (cameraX < 0) cameraX = 0;
    if (cameraX > maxCameraX) cameraX = maxCameraX;



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
                gameState = LEVEL_SELECT;   // <-- go to new screen instead of GAME
            }
            //Score


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
                loadLevelFromFile(currentLevel);  // <-- ADD HERE
                gameState = GAME;
            }
            // Level-2 button
            else if (mx >= 427 && mx <= 740 && my >= 194 && my <= 244)
            {
                currentLevel = 2;
                loadLevelFromFile(currentLevel);  // <-- ADD HERE
                gameState = GAME;
            }
            // Level-3 button
            else if (mx >= 58 && mx <= 369 && my >= 100 && my <= 151)
            {
                currentLevel = 3;
                loadLevelFromFile(currentLevel);  // <-- ADD HERE
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
        else if (gameState == LEVEL_COMPLETE)
{
    // Next Level button
    if (mx >= 280 && mx <= 480 && my >= 200 && my <= 250)
    {
        if (currentLevel < maxLevel)
        {
            currentLevel++;
            loadLevelFromFile(currentLevel);
            pic_x = 320;
            pic_y = 90;
            cameraX = 0;
            gameState = GAME;
        }
        else
        {
            gameState = GAME_WIN;
        }
    }
    // Back to Menu button
    else if (mx >= 280 && mx <= 480 && my >= 130 && my <= 180)
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
    if (mx >= 300 && mx <= 500 && my >= 320 && my <= 360)
    {
        gameState = GAME;
        isPaused = false;
    }
    // New Game
    else if (mx >= 300 && mx <= 500 && my >= 260 && my <= 300)
    {
        // Reset everything for new game
        currentLevel = 1;
        loadLevelFromFile(currentLevel);
        pic_x = 320;
        pic_y = 90;
        cameraX = 0;
        coinCount = 0;
        score = 0;
        gameOver = false;
        gameStartTime = time(NULL);
        gameState = GAME;
        isPaused = false;
    }
    // Settings (optional)
    else if (mx >= 300 && mx <= 500 && my >= 200 && my <= 240)
    {
        // Just a placeholder for now
        printf("Settings clicked\n");
    }
    // Main Menu
    else if (mx >= 300 && mx <= 500 && my >= 140 && my <= 180)
    {
        gameState = MENU;
        isPaused = false;
    }
}



        // For debug (optional)
        printf(" %d %d\n", mx, my);
    }
}

/*
if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
{
    // place your codes here
}
*/

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
        if (key == 13)   // Enter
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
        if (key == ' ' && !gameOver)
        {
            if (!jump)
            {
                jump = 1;
                jumpSpeed = jumpHeight;
            }
        }
        else if (key == 'r' || key == 'R')
        {
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
    else if (gameState == GAME_WIN)
    {
        if (key == 'm' || key == 'M')
        {
            gameState = MENU;
        }
        else if (key == 'e' || key == 'E')
        {
            exit(0);
        }
    }
    else if (gameState == LEVEL_COMPLETE)
{
    if (key == 'n' || key == 'N')
    {
        if (currentLevel < maxLevel)
        {
            currentLevel++;
            loadLevelFromFile(currentLevel);
            pic_x = 320;
            pic_y = 90;
            cameraX = 0;
            gameState = GAME;
        }
        else
        {
            gameState = GAME_WIN;
        }
    }
    else if (key == 'm' || key == 'M')
    {
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
void iSpecialKeyboard(unsigned char key)
{
    
    switch (key)
    {
    case GLUT_KEY_UP:
        iIncreaseVolume(bgSoundIdx, 5);
        break;
    case GLUT_KEY_DOWN:
        iDecreaseVolume(bgSoundIdx, 5);
        break;
    // place your codes for other keys here
    default:
        break;
    }
    if (key == GLUT_KEY_RIGHT)
    {
        rightPressed = true;
        leftPressed = false;  // Optional: prevent simultaneous left+right
    }
    else if (key == GLUT_KEY_LEFT)
    {
        leftPressed = true;
        rightPressed = false;
    }



}
/*
void iSpecialKeyboardUp(unsigned char key) {
    if (key == GLUT_KEY_RIGHT)
        rightPressed = false;
    else if (key == GLUT_KEY_LEFT)
        leftPressed = false;
}
*/
/*
void iAnim(){
    if(jump){
        iAnimateSprite(&golem);
        return;
    }

    if(direction==-1){
        golem.x-=3;
        speed=0;
        activity(1);
    }
    else if(direction == 1){
        if(golem.x>350){
            speed=-3;
            tile_x=tile_x+speed;
        }
        else{
            golem.x+=3;
        }
        activity(1);
    }
    else{
        activity(0);
    }

    iAnimateSprite(&golem);
}
*/

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
   
    // place your own initialization codes here.
    populate_run_images();
    loadResources();
    iSetTimer(100, update_run);
    iSetTimer(20, iUpdate); // Call iUpdate every 20 ms

    // frontPageImage = iLoadImage("c:/Users/user/OneDrive/Desktop/Game Project Pic/1st Cover003.bmp");
    //iLoadImage(&frontPageImage, "c:/Users/user/OneDrive/Desktop/Game Project Pic/1st Cover003.bmp");
    iInitializeSound();
    bgSoundIdx = iPlaySound("assets/sounds/background.wav", true, 50);
    iInitialize(800, 500, "Super Mario");
    return 0;
}