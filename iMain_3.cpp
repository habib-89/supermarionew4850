#include "iGraphics.h"

/*
function iDraw() is called again and again by the system.
*/

int pic_x, pic_y;
int golem_idx = 0;
int jump_idx = 0;
// char golem_idle[17][100];
// char golem_jump[11][100];
int jump = 0;
int jump_speed = 0;
int gravity = 1;
int jump_height = 15;
int ground = 122;
int direction = 0;
int tile_x = 0, tile_y = ground;
int tile_width = 37, tile_height = 35;
Image bg;
Image golem_idle[2];
Image golem_run_frames[7];
Image golem_jump_frames[11];
Image tile_set[1];
Sprite tiles[100];
Sprite golem;
int speed = 0;
int golem_running = 0;
int animation = -1; // 0 idle, 1 run, 2 jump
bool going_right = true;
bool gameOver = false;
#define MENU 0
#define GAME 1
#define GAME_OVER 2
#define FRONT_PAGE -1
int gameState = FRONT_PAGE;
int gameStartTime = 0;

// char tile_array[100][100]= {"________*_______________________________________________________"};

char tile_array[6][68] = {
    "___________________________________________________________________",
    "___________________________________________________________________",
    "___________________________________*****___________________________",
    "__________________*_________***________________**__________________",
    "___________*______*_____*________________***________*______________",
    "_____*_____*______*_________________________________**_____*___*___"};

int tile_idx = 0;
int scroll_x = 0;

void load_bg()
{
    iLoadImage(&bg, "assets/images/background.jpg");
    iResizeImage(&bg, 900, 450);
}

void loadResources()
{
    iLoadFramesFromFolder(golem_idle, "Game Project Pic/walk");
    iLoadFramesFromFolder(golem_run_frames, "Game Project Pic/run");
    iLoadFramesFromFolder(golem_jump_frames, "Game Project Pic/jump");
    iInitSprite(&golem, -1);
    iChangeSpriteFrames(&golem, golem_idle, 1);
    iSetSpritePosition(&golem, 70, 122);

    // new code

    iLoadFramesFromFolder(tile_set, "Game Project Pic/tiile");
    const int h = ground + tile_height * 5;
    tile_y = ground + tile_height * 5;
    for (int i = 0; i < 6; i++)
    {
        tile_y = h - i * tile_height;
        tile_x = 0;
        for (int j = 0; j < 68; j++)
        {
            if (tile_array[i][j] == '_')
                tile_x += tile_width;
            else if (tile_array[i][j] == '*')
            {
                iInitSprite(&tiles[tile_idx], -1);
                iChangeSpriteFrames(&tiles[tile_idx], tile_set, 1);
                iSetSpritePosition(&tiles[tile_idx], tile_x, tile_y);
                tile_x += tile_width;
                tile_idx++;
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
            iChangeSpriteFrames(&golem, golem_jump_frames, 11);
    }
}

int collision(Sprite *s)
{
    for (int i = 0; i < tile_idx; i++)
    {
        if (iCheckCollision(s, &tiles[i]))
        {
            return i;
        }
    }
    return -1;
}

void update_jump()
{
    // bool on_ground = collision(&golem);
    int idx = collision(&golem);
    if (jump)
    {
        golem.y += jump_speed;
        // new code
        if (direction == 1 && golem.x<350)
        {
            golem.x += 3;
        }
        else if (direction == -1 &&golem.x<350)
        {
            golem.x -= 3;
        }
        //
        jump_speed -= gravity;
        if (idx != -1)
        {
            golem.y = tiles[idx].y + tile_height;
            jump = 0;
            jump_speed = 0;
            activity(1);
        }
        if (golem.y <= ground)
        {
            golem.y = ground;
            jump = 0;
            jump_speed = 0;
            activity(1);
        }
    }
    else
    {
        if (idx == -1 && golem.y > ground)
        {
            jump = 1;
            jump_speed = 0;
            // on_ground = false;
            activity(1);
        }
        else if (golem.y == ground || collision(&golem))
        {
            // on_ground = true;
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
        iShowImage(0, 0, "Game Project Pic/1st Cover002.png");
        iSetColor(0, 0, 0);
        iText(10, 10, "Press Enter to Continue or Click Main Menu", GLUT_BITMAP_HELVETICA_18);
    }

    else if (gameState == MENU)
    {
        iShowImage(0, 0, "Game Project Pic/2nd Cover002.bmp");
    }
    else if (gameState == GAME)
    {
        iShowLoadedImage(0, 0, &bg);
        iWrapImage(&bg, speed);
        iShowSprite(&golem);
        // new code
        for (int i = 0; i < tile_idx; i++)
        {
            iShowSprite(&tiles[i]);
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
            // Click "Main Menu" button on front page
            if (mx >= 467 && mx <= 750 &&
                my >= 27 && my <= 66)
            {
                gameState = MENU;
            }
        }
        else if (gameState == MENU)
        {
            // Check if click is inside the start button
            if (mx >= 540 && mx <= 760 &&
                my >= 305 && my <= 360)
            {
                gameState = GAME;
                gameOver = false;
                gameStartTime = time(NULL);
            }
        }
        printf("%d %d ", mx, my);
    }

    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        // place your codes here
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
        if (key == 13)
        { // Enter
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
    if (jump)
    {
        iAnimateSprite(&golem);
        return;
    }
    int touch = collision(&golem);
    if (direction == -1)
    {
        golem.x -= 3;
        speed = 0;
        activity(1);
        // new code
        if (touch != -1)
        {
            golem.x += 3;
        }
        //
    }
    else if (direction == 1)
    {
        if (golem.x > 350)
        {
            speed = -3;
        }
        else
        {
            golem.x += 3;
        }
        // new code
        if (touch!= -1)
        {
            golem.x -= 3;
            //
        }
        else
        {
            activity(1);
        }
    }
    else activity(0);
    
    iAnimateSprite(&golem);
}
void animate_tile()
{
    if (direction == 1 && golem.x > 350 )
    {
        for (int i = 0; i < tile_idx; i++)
        {
            tiles[i].x -= 5;
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
    iSetTimer(60, animate_tile);
    iSetTimer(20, update_jump);
    iInitialize(900, 450, "demo");

    return 0;
}

// 69, 37, 150