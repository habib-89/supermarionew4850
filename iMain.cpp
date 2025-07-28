#include "iGraphics.h"
#include <iostream>
#include <fstream>
#include "iSound.h"
using namespace std;

#define MENU 0
#define GAME 1
#define GAME_OVER 2
#define FRONT_PAGE 3
#define HELP 4
#define LEVEL_SELECT 5
#define PAUSE_MENU 6
#define LEVEL_COMPLETE 7
#define GAME_OVER_SCREEN 8
#define SCORE 9
#define SAVE_SLOT_SELECT 10
#define LOAD_SLOT_SELECT 11
#define GAME_COMPLETED 12
#define MAP_WIDTH 200
#define MAP_HEIGHT 17
#define GROUND 122
#define MAX_RESPWAN_TIMER 50
#define MAX_HURT_TIMER 50
#define MAX_DEAD_TIMER 50
#define MAX_SCORES 5

int level1Score = 0;
int level2Score = 0;
int level3Score = 0;

int golem_height = 76;
int golem_width = 45;

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
int spike_width = 60, spike_height = 30;

Image bg[4];
Image golem_idle[5];
Image golem_run_frames[7];
Image golem_jump_frames[11];
Image golem_hurt_frames[3];
Image golem_dead_frames[3];
Image tile_set[3];
Sprite tiles[MAP_HEIGHT * MAP_WIDTH];
Sprite golem;
Sprite flag;
Image heart_full, heart_empty;

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

bool saveFileExists[3] = {false, false, false};
int selectedSlot = -1;

int currentLevel = 1;
const int maxLevel = 3;
bool levelCompleted[3] = {false, false, false};

int score = 0;
int frame = 0;

int lastBrickX = 0;
bool levelComplete = false;

int life = 3;
bool hurt = false;
int hurtTimer = 0;
bool dead = false;
int deadTimer = 0;
bool respawn = false;
int respawn_timer = 0;

char playerName[100] = "";
char highScorer[100] = "None";
int highScore = 0;
bool enteringNameHigh = false;
bool enteringNameLow = false;
int nameCharIdx = 0;

char highScorers[MAX_SCORES][100];
int highScores[MAX_SCORES];

void loadLevelFromFile(int level)
{
    char filename[100];
    sprintf(filename, "assets/levels(rafsan)/level_%d.txt", level);

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

    iLoadImage(&tile_set[0], "assets/GameBG/Brick_01.png");
    iLoadImage(&tile_set[1], "assets/GameBG/Coin003.png");
    iLoadImage(&tile_set[2], "assets/GameBG/Spikes.png");

    Image flagImg;
    iLoadImage(&flagImg, "assets/GameBG/Win Flag002.png");

    tile_idx = 0;
    tile_x = 0;
    lastBrickX = 0;
    const int h = tile_height * (MAP_HEIGHT - 1);

    for (int i = 0; i < MAP_HEIGHT; i++)
    {
        tile_y = h - tile_height * i;
        tile_x = 0;
        for (int j = 0; j < MAP_WIDTH; j++)
        {
            char current = tileMap[i][j];

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

void checkSaveFiles()
{
    for (int i = 0; i < 3; i++)
    {
        char filename[100];
        sprintf(filename, "saves/savegame%d.txt", i + 1);
        FILE *fp = fopen(filename, "r");
        if (fp)
        {
            saveFileExists[i] = true;
            fclose(fp);
        }
        else
        {
            saveFileExists[i] = false;
        }
    }
}

void load_bg()
{
    iLoadImage(&bg[1], "assets/Level1image/BGL1001.png");
    iLoadImage(&bg[2], "assets/GameBG/Level2BG.jpg");
    iLoadImage(&bg[3], "assets/GameBG/LEVEL3BG02.png");
}

void startLevel(int level)
{
    if (level==1)
    {
        score=0;
    }
    life = 3;
    hurt = false;
    hurtTimer = 0;
    dead = false;
    deadTimer = 0;
    tile_idx = 0;
    // for (int i = 0; i < MAP_HEIGHT * MAP_WIDTH; i++)
    // {
    //     tile_type[i] = '_';
    // }

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

void loadHighScores()
{
    FILE *fp = fopen("assets/Saves/Highscore.txt", "r");
    int i = 0;

    if (fp != NULL)
    {
        while (fscanf(fp, "%s %d", highScorers[i], &highScores[i]) == 2 && i < MAX_SCORES)
            i++;
        fclose(fp);
    }

    for (int j = i; j < MAX_SCORES; j++)
    {
        strcpy(highScorers[j], "---");
        highScores[j] = 0;
    }
    highScore = highScores[0];
    strcpy(highScorer, highScorers[0]);

}

void saveHighScore(char *playerName, int playerScore)
{
    int i, inserted = 0;

    for (i = 0; i < MAX_SCORES; i++)
    {
        if (playerScore > highScores[i])
        {
            for (int j = MAX_SCORES - 1; j > i; j--)
            {
                highScores[j] = highScores[j - 1];
                strcpy(highScorers[j], highScorers[j - 1]);
            }

            highScores[i] = playerScore;
            strcpy(highScorers[i], playerName);
            inserted = 1;
            break;
        }
    }

    if (inserted)
    {
        FILE *fp = fopen("assets/Saves/Highscore.txt", "w");
        for (int k = 0; k < MAX_SCORES; k++)
        {
            fprintf(fp, "%s %d\n", highScorers[k], highScores[k]);
        }
        fclose(fp);
    }
}

void saveGameState(int slot)
{
    char filename[100];
    sprintf(filename, "saves/savegame%d.txt", slot + 1);
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("Failed to open %s for writing\n", filename);
        return;
    }

    fprintf(fp, "%d\n", currentLevel);
    fprintf(fp, "%d %d\n", golem.x, golem.y);
    fprintf(fp, "%d\n", score);
    fprintf(fp, "%d\n", life);
    fprintf(fp, "%d\n", jump);
    fprintf(fp, "%d\n", jump_speed);
    fprintf(fp, "%d\n", direction);
    fprintf(fp, "%d\n", gameStartTime);
    fprintf(fp, "%d\n", scroll_x);
    fprintf(fp, "%d\n", (int)gameOver);
    fprintf(fp, "%d\n", animation);
    fprintf(fp, "%d\n", frame);
    fprintf(fp, "%d\n", tile_idx);
    for (int i = 0; i < tile_idx; i++)
    {
        fprintf(fp, "%d %d %c\n", tiles[i].x, tiles[i].y, tile_type[i]);
    }
    fprintf(fp, "%d %d\n", flag.x, flag.y);

    fclose(fp);
    saveFileExists[slot] = true;
    // printf("Game state saved to %s\n", filename);
}

void loadGameState(int slot)
{
    char filename[100];
    sprintf(filename, "saves/savegame%d.txt", slot + 1);
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("No saved game found in %s.\n", filename);
        saveFileExists[slot] = false;
        return;
    }

    int tempGameOver, tempGoingRight;
    fscanf(fp, "%d", &currentLevel);
    fscanf(fp, "%d %d", &golem.x, &golem.y);
    fscanf(fp, "%d", &score);
    fscanf(fp, "%d", &life);
    fscanf(fp, "%d", &jump);
    fscanf(fp, "%d", &jump_speed);
    fscanf(fp, "%d", &direction);
    fscanf(fp, "%d", &gameStartTime);
    fscanf(fp, "%d", &scroll_x);
    fscanf(fp, "%d", &tempGameOver);
    fscanf(fp, "%d", &animation);
    fscanf(fp, "%d", &frame);
    fscanf(fp, "%d", &tile_idx);

    gameOver = (bool)tempGameOver;

    for (int i = 0; i < tile_idx; i++)
    {
        int x, y;
        char type;
        fscanf(fp, "%d %d %c", &x, &y, &type);
        iInitSprite(&tiles[i]);
        int tileIndex = (type == '*') ? 0 : (type == 'o') ? 1 : 2;
        iChangeSpriteFrames(&tiles[i], &tile_set[tileIndex], 1);
        iSetSpritePosition(&tiles[i], x, y);
        tile_type[i] = type;

    }


    int flag_x, flag_y;
    fscanf(fp, "%d %d", &flag_x, &flag_y);
    Image flagImg;
    iLoadImage(&flagImg, "assets/GameBG/Win Flag002.png");
    iInitSprite(&flag);
    iChangeSpriteFrames(&flag, &flagImg, 1);
    iSetSpritePosition(&flag, flag_x, flag_y);

    fclose(fp);

    iSetSpritePosition(&golem, golem.x, golem.y);
    activity(animation);
    golem.currentFrame = frame;

    if (!bg[currentLevel].data)
    {
        load_bg();
    }

    saveFileExists[slot] = true;
    gameState = GAME;
    isPaused = false;
    levelComplete = false;
    printf("Game state loaded from %s\n", filename);
}

void loadResources()
{
    iLoadFramesFromFolder(golem_idle, "assets/idle");
    iLoadFramesFromFolder(golem_run_frames, "assets/Run");
    iLoadFramesFromFolder(golem_jump_frames, "assets/Game Project Pic/jump");
    iLoadFramesFromFolder(golem_hurt_frames, "assets/hurt");
    iLoadFramesFromFolder(golem_dead_frames, "assets/dead");
    iInitSprite(&golem);
    iChangeSpriteFrames(&golem, golem_idle, 1);
    iSetSpritePosition(&golem, 70, 122);
    iLoadImage(&heart_full, "assets/Heart/HeartRed.png");
    iLoadImage(&heart_empty, "assets/Heart/HeartBlack.png");
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

bool horizontal_collision(int new_x, int golem_y, int golem_width, int golem_height)
{
    Sprite test = golem;
    test.x = new_x;
    test.y = golem_y;

    for (int i = 0; i < tile_idx; i++)
    {
        if (tile_type[i] != '*')
            continue;
        if (iCheckCollision(&test, &tiles[i]))
        {
            return true;
        }
    }
    return false;
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
                if (golem.y + 10 > tiles[idx].y + tile_height)
                {
                    golem.y = tiles[idx].y + tile_height;
                    jump = 0;
                    jump_speed = 0;
                    activity(1);
                }
                else
                {
                    if (direction == 1)
                    {
                        golem.x -= 20;
                        direction = 0;
                    }
                    else if (direction == -1)
                    {
                        golem.x += 20;
                        direction = 0;
                    }
                }
            }
            else
            {
                if (golem.y + golem_height - 10 < tiles[idx].y)
                {
                    golem.y = tiles[idx].y - golem_height;
                    jump_speed = 0;
                }
                else
                {
                    if (direction == 1)
                    {
                        golem.x -= 20;
                        direction = 0;
                    }
                    else if (direction == -1)
                    {
                        golem.x += 20;
                        direction = 0;
                    }
                }
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

    for (int i = 0; i < tile_idx; i++)
    {
        if (tile_type[i] == 'o')
        {
            if (iCheckCollision(&golem, &tiles[i]))
            {
                tile_type[i] = '_';
                score += 10;
                iPlaySound("assets/sounds/Coin.wav", false, 50);
            }
        }
    }

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
                iPlaySound("assets/sounds/hurt.wav", false, 50);
                // if (golem.x < tiles[i].x)
                //     golem.x = tiles[i].x - tile_width;
                // else
                //     golem.x = tiles[i].x + tile_width;
                if (life > 0)
                {
                    activity(3);
                }
                else
                {
                    // hurt = false;
                    dead = true;
                    activity(4);
                    frame = 0;
                    deadTimer = 0;
                }

            }
        }
    }

    if (!levelComplete && iCheckCollision(&golem, &flag))
    {
        levelComplete = true;
        direction = 0;
        speed = 0;
        gameState = LEVEL_COMPLETE;
        levelCompleted[currentLevel - 1] = true;
    }

    if (hurt && !dead)
    {
        hurtTimer++;
        if (hurtTimer > MAX_HURT_TIMER)
        {
            hurt = false;
            hurtTimer = 0;
        }
    }

    if (golem.y + golem_height < 0)
    {
        direction = 0;
        respawn_timer++;
        if (respawn_timer >= MAX_RESPWAN_TIMER)
        {
            life--;
            respawn = false;
            respawn_timer = 0;
            if (life > 0)
            {
                golem.y = 400;
                golem.x += tile_width * 6;
                direction = 0;
            }
            else
            {
                dead = true;
                deadTimer = 0;
                direction = 0;
                gameState = GAME_OVER_SCREEN;
            }
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
                enteringNameHigh = true;
                nameCharIdx = 0;
                playerName[0] = '\0';
            }
            else
            {
                enteringNameLow = true;
                nameCharIdx = 0;
                playerName[0] = '\0';
            }
        }
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
    else if (gameState == MENU)
    {
        iShowImage(0, 0, "assets/GameBG/2nd cover004.png");

    }
    else if (gameState == SCORE)
    {
        iClear();
        iShowImage(0, 0, "assets/GameBG/scorebg001.png");

        for (int i = 0; i < MAX_SCORES; i++)
        {
            char line[200];
            sprintf(line, "%s - %d", highScorers[i], highScores[i]);
            iSetColor(232, 195, 203);
            iText(295, 257 - i * 45, line, GLUT_BITMAP_HELVETICA_18);
        }

        iSetColor(200, 200, 200);
        iFilledRectangle(40, 40, 100, 40);
        iSetColor(0, 0, 0);
        iText(55, 55, "Back", GLUT_BITMAP_HELVETICA_18);
    }
    else if (gameState == HELP)
    {
        iClear();
        iShowImage(0, 0, "assets/GameBG/Help Cover001.png");

        iSetColor(0, 0, 0);
        iText(300, 450, "HELP", GLUT_BITMAP_TIMES_ROMAN_24);

        iText(160, 370, "1. Use RIGHT and LEFT arrow keys to move");
        iText(160, 340, "2. Press UP arrow key to jump");
        iText(160, 310, "3. Press DOWN arrow key to stop");
        iText(160, 280, "4. Avoid obstacles to survive");
        iText(160, 250, "5. Press 'R' to restart the game after Game Over");
        iText(160, 220, "6. Press 'E' to exit the game");
        iText(160, 190, "7. Press 'S' to save game (in pause menu)");
        iText(160, 160, "8. Press 'L' to load game (in pause menu)");

        iSetColor(100, 100, 100);
        iFilledRectangle(230, 20, 100, 35);
        iSetColor(255, 255, 255);
        iText(265, 35, "Back");
    }
    else if (gameState == LEVEL_SELECT)
    {
        iClear();
        iShowImage(0, 0, "assets/GameBG/Level BG001.png");
        if (!levelCompleted[0])
            iShowImage(468,204,"assets/GameBG/lock image001.png");

        if (!levelCompleted[1])
            iShowImage(100,110,"assets/GameBG/lock image001.png");

    }
    else if (gameState == GAME)
    {
        iClear();
        iShowLoadedImage(0, 0, &bg[currentLevel]);
        iShowImage(730,760,"assets/GameBG/pausebutton001.png");

        iShowSprite(&golem);

        if (direction == 1 && golem.x >= 350)
        {
            iWrapImage(&bg[currentLevel], -2);
        }
        char scoreStr[50];
        sprintf(scoreStr, "Score: %d", score);
        iSetColor(255, 255, 255);
        iText(20, 460, scoreStr, GLUT_BITMAP_HELVETICA_18);

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
                iShowLoadedImage(20 + i * 35, 400, &heart_full);
            else
                iShowLoadedImage(20 + i * 35, 400, &heart_empty);
        }

        iShowSprite(&flag);
        iShowImage(730,440,"assets/GameBG/pausebutton001.png");
    }
    else if (gameState == PAUSE_MENU)
    {
        iClear();
        iShowImage(0, 0, "assets/GameBG/Pause 003.png");

    }
    else if (gameState == SAVE_SLOT_SELECT)
    {
        iClear();
        iShowImage(0, 0, "assets/GameBG/Selectslot001.png");

    }
    else if (gameState == LOAD_SLOT_SELECT)
    {
        iClear();
        iShowImage(0, 0, "assets/GameBG/Selectslot001.png");

    }
    else if (gameState == GAME_OVER_SCREEN)
    {
        iClear();
        iShowImage(0, 0, "assets/GameBG/Gameoverbg001.png");
    }
    else if (gameState == LEVEL_COMPLETE)
    {
        iClear();

        if (currentLevel == 1)
            level1Score = score;
        else if (currentLevel == 2)
            level2Score = score - level1Score;
        else if (currentLevel == 3)
            level3Score = score - level1Score - level2Score;

        if (currentLevel == 3)
        {
            gameState = GAME_COMPLETED;
        }
        else
        {
            iShowImage(0, 0, "assets/GameBG/Level Complete001.png");
        }

    }
    else if (gameState == GAME_COMPLETED)
    {
        iClear();
        iSetColor(38, 110, 110);
        iText(300, 400, "All Levels Completed!", GLUT_BITMAP_TIMES_ROMAN_24);

        char scoreText[100];
        sprintf(scoreText, "Your Total Score: %d", score);
        iText(300, 350, scoreText, GLUT_BITMAP_TIMES_ROMAN_24);

        iText(300, 300, "Enter Your Name:", GLUT_BITMAP_HELVETICA_18);
        iRectangle(300, 260, 200, 30);
        iText(310, 268, playerName, GLUT_BITMAP_HELVETICA_18);

        if (score > highScore)
            iText(300, 220, "New High Score!", GLUT_BITMAP_TIMES_ROMAN_24);

        iText(300, 170, "Press Enter to Save", GLUT_BITMAP_HELVETICA_18);
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

void iMouseMove(int mx, int my)
{
}

void iMouseDrag(int mx, int my)
{
}

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
            if (mx >= 138 && mx <= 630 && my >= 340 && my <= 402)
            {
                gameState = LEVEL_SELECT;
            }
            // Load Game
            else if (mx >= 138 && mx <= 630 && my >= 258 && my <= 320)
            {
                gameState = LOAD_SLOT_SELECT;
            }
            // score
            else if (mx >= 138 && mx <= 630 && my >= 180 && my <= 238)
            {
                gameState = SCORE;
            }
            // help
            else if (mx >= 138 && mx <= 630 && my >= 100 && my <= 158)
            {
                gameState = HELP;
            }
            // Exit
            else if (mx >= 138 && mx <= 630 && my >= 40 && my <= 95)
            {
                exit(0);
            }
        }
        else if (gameState == SCORE)
        {
            if (mx >= 40 && mx <= 140 && my >= 40 && my <= 80)
            {
                gameState = MENU;
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
                if (levelCompleted[0])
                {
                    currentLevel = 2;
                    startLevel(currentLevel);
                }
            }
            else if (mx >= 58 && mx <= 369 && my >= 100 && my <= 151)
            {
                if (levelCompleted[1])
                {
                    currentLevel = 3;
                    startLevel(currentLevel);
                }
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
            if (mx >= 730 && mx <= 780 && my >= 438 && my <= 490)
            {
                gameState = PAUSE_MENU;
                isPaused = true;
            }
        }
        else if (gameState == PAUSE_MENU)
        {
            if (mx >= 250 && mx <= 500 && my >= 310 && my <= 356)
            {
                gameState = GAME;
                isPaused = false;
            }
            else if (mx >= 250 && mx <= 500 && my >= 253 && my <= 299)
            {
                currentLevel = 1;
                startLevel(currentLevel);
            }

            else if (mx >= 250 && mx <= 500 && my >= 195 && my <= 241)
            {
                gameState = SAVE_SLOT_SELECT;
            }
            else if (mx >= 250 && mx <= 500 && my >= 137 && my <= 185)
            {
                gameState = LOAD_SLOT_SELECT;
            }
            else if (mx >= 250 && mx <= 500 && my >= 80 && my <= 126)
            {
                gameState = MENU;
                isPaused = false;
            }
        }
        else if (gameState == SAVE_SLOT_SELECT)
        {
            if (mx >= 222 && mx <= 532 && my >= 298 && my <= 368)
            {
                saveGameState(0);
                gameState = PAUSE_MENU;
            }
            else if (mx >= 222 && mx <= 532 && my >= 210 && my <= 278)
            {
                saveGameState(1);
                gameState = PAUSE_MENU;
            }
            else if (mx >= 222 && mx <= 532 && my >= 120 && my <= 189)
            {
                saveGameState(2);
                gameState = PAUSE_MENU;
            }
            else if (mx >= 290 && mx <= 465 && my >= 42 && my <= 99)
            {
                gameState = PAUSE_MENU;
            }
        }
        else if (gameState == LOAD_SLOT_SELECT)
        {
            if (mx >= 222 && mx <= 532 && my >= 298 && my <= 368 && saveFileExists[0])
            {
                loadGameState(0);
            }
            else if (mx >= 222 && mx <= 532 && my >= 210 && my <= 278 && saveFileExists[1])
            {
                loadGameState(1);
            }
            else if (mx >= 222 && mx <= 532 && my >= 120 && my <= 189 && saveFileExists[2])
            {
                loadGameState(2);
            }
            else if (mx >= 290 && mx <= 465 && my >= 42 && my <= 99)
            {
                gameState = gameState == PAUSE_MENU ? PAUSE_MENU : MENU;
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
            else if (mx >= 418 && mx <= 608 && my >= 68 && my <= 138)
            {
                gameState = MENU;
            }
        }
        else if (gameState == GAME_OVER_SCREEN)
        {
            if (mx >= 185 && mx <= 380 && my >= 53 && my <= 115)
            {
                if (currentLevel == 1)
                    score = 0;
                else if (currentLevel == 2)
                    score = level1Score;
                else if (currentLevel == 3)
                    score = level1Score + level2Score;

                life = 3;
                startLevel(currentLevel);
                gameState = GAME;
            }
            else if (mx >= 415 && mx <= 608 && my >= 53 && my <= 115)
            {
                gameState = MENU;
            }
        }

        printf(" %d %d\n", mx, my);
    }
}

void iMouseWheel(int dir, int mx, int my)
{

}
void iKeyboard(unsigned char key, int state)
{
    if (gameState == FRONT_PAGE)
    {
        if (key == 13 && state == GLUT_DOWN)
        {
            gameState = MENU;
        }
    }
    else if (gameState == MENU)
    {
        if ((key == 'e' || key == 'E') && state == GLUT_DOWN)
        {
            exit(0);
        }
        else if ((key == 'l' || key == 'L') && state == GLUT_DOWN)
        {
            gameState = LOAD_SLOT_SELECT;
        }
    }
    else if (gameState == GAME)
    {
        if ((key == 'r' || key == 'R') && state == GLUT_DOWN)
        {
            gameOver = false;
            life = 3;
            startLevel(currentLevel);
        }
        else if ((key == 'e' || key == 'E') && state == GLUT_DOWN)
        {
            exit(0);
        }
        else if ((key == 's' || key == 'S') && state == GLUT_DOWN)
        {
            gameState = SAVE_SLOT_SELECT;
        }
        else if ((key == 'l' || key == 'L') && state == GLUT_DOWN)
        {
            gameState = LOAD_SLOT_SELECT;
        }
    }
    else if (gameState == PAUSE_MENU)
    {
        if ((key == 'p' || key == 'P') && state == GLUT_DOWN)
        {
            gameState = GAME;
            isPaused = false;
        }
        else if ((key == 's' || key == 'S') && state == GLUT_DOWN)
        {
            gameState = SAVE_SLOT_SELECT;
        }
        else if ((key == 'l' || key == 'L') && state == GLUT_DOWN)
        {
            gameState = LOAD_SLOT_SELECT;
        }
    }


    if (gameState == GAME_COMPLETED && state == GLUT_DOWN)
    {
        if (key == '\b')
        {
            int len = strlen(playerName);
            if (len > 0)
            {
                playerName[len - 1] = '\0';
            }
        }
        else if (key == '\r')
        {
            saveHighScore(playerName, score);
            gameState = SCORE;
        }
        else if (key >= 32 && key <= 126 && strlen(playerName) < 15)
        {
            int len = strlen(playerName);
            playerName[len] = key;
            playerName[len + 1] = '\0';
        }
    }

    if (state == GLUT_DOWN)
    {
        if (key == 'c')
            iResumeSound(bgSoundIdx);
        else if (key == 'p')
            iPauseSound(bgSoundIdx);
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
        iIncreaseVolume(bgSoundIdx, 2);
        break;
    case GLUT_KEY_DOWN:
        iDecreaseVolume(bgSoundIdx, 2);
        break;
    default:
        break;
    }
}

void iAnim()
{
    if (gameState != GAME)
        return;
    int idx;

    if (dead)
    {
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

    Sprite test = golem;

    if (direction == -1)
    {
        int new_x = golem.x - golemSpeed;
        if (new_x >= 0 && !horizontal_collision(new_x, golem.y, golem_width, golem_height))
        {
            golem.x = new_x;
            speed = 0;
            activity(1);
        }
        else
        {
            direction = 0;
            activity(0);
        }
    }
    else if (direction == 1)
    {
        if (golem.x + scroll_x >= lastBrickX + tile_width + 150)
        {
            direction = 0;
            activity(0);
            return;
        }

        int new_x = golem.x + golemSpeed;
        if (!horizontal_collision(new_x, golem.y, golem_width, golem_height))
        {
            if (golem.x > 350)
            {
                speed = -golemSpeed;
            }
            else
            {
                golem.x = new_x;
                speed = 0;
            }
            activity(1);
        }
        else
        {
            direction = 0;
            activity(0);
        }
    }
    else
    {
        speed = 0;
        activity(0);
    }

    iAnimateSprite(&golem);
}

void animate_tile()
{
    if (gameState != GAME)
        return;
    if (direction == 1 && golem.x >= 350)
    {
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
    }
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    loadResources();
    loadHighScores();
    load_bg();
    checkSaveFiles();
    loadLevelFromFile(1);
    iSetTimer(80, iAnim);
    iSetTimer(19, animate_tile);
    iSetTimer(30, update_jump);

    iInitializeSound();
    bgSoundIdx = iPlaySound("assets/sounds/Platformer Bonus level.wav", true, 30);

    iOpenWindow(800, 500, "Super BUET Bros");

    return 0;
}

