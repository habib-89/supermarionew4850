#include "iGraphics.h"
#include <stdbool.h>  // for bool

int pic_x=320, pic_y=100;
int idle_idx = 0;
char run_idle[18][100];  
// int bgImage;

#define NUM_OBSTACLES 5

int obsX[NUM_OBSTACLES] = {1000, 1500, 2000, 2500, 3000};   // X positions, start farther away
int obsY = 100;                                            // Y position for all
int obsWidth = 30;
int obsHeight = 50;

// Character and physics
int groundY = 100;
int jump = 0;
int jumpSpeed = 0;
int gravity = 1;
int jumpHeight = 15;
int moveSpeed = 10;

// Game state
bool gameOver = false;

int score = 0;
char scoreText[20];


void populate_run_images()
{
    for (int i = 0; i < 8; i++)
    {
        sprintf(run_idle[i], "c:/Users/user/OneDrive/Desktop/Game Project Pic/Run%03d.png", i);
    }
}

void update_run()
{
    idle_idx = (idle_idx + 1) % 8;
}

bool checkCollision(int charX, int charY, int obsX, int obsY, int obsW, int obsH) {
    return !(charX + 50 < obsX || charX > obsX + obsW || charY + 80 < obsY || charY > obsY + obsH);
}

void iDraw()
{
    iClear();

    // Background
    iSetTransparentColor(89, 156, 106, 0.5);
    iFilledRectangle(0, 0, 800, 500);

    // Ground
    iSetColor(56, 21, 3);
    iFilledRectangle(0, 0, 800, 100);

    // Sun
    iSetColor(255, 255, 0);
    iFilledCircle(700, 420, 50);

    // Obstacles - all same (red blocks)
    iSetColor(41, 17, 9);
    for (int i = 0; i < NUM_OBSTACLES; i++) {
        iFilledRectangle(obsX[i], obsY, obsWidth, obsHeight);
    }

    // Character sprite
    iShowImage(pic_x, pic_y, run_idle[idle_idx]);

    // If game over, show message
    if (gameOver) {
        iSetColor(255, 0, 0);
        iText(300, 250, "GAME OVER! Press R to Restart", GLUT_BITMAP_HELVETICA_18);

        sprintf(scoreText, "Score: %d", score);
iSetColor(0, 0, 0);
iText(20, 470, scoreText, GLUT_BITMAP_HELVETICA_18);

    }
}

void iUpdate() {
    if (gameOver) return;  // Stop game updates on collision

    if (jump) {
        pic_y += jumpSpeed;
        jumpSpeed -= gravity;

        if (pic_y <= groundY) {
            pic_y = groundY;
            jump = 0;
            jumpSpeed = 0;
        }
    }

    for (int i = 0; i < NUM_OBSTACLES; i++) {
        obsX[i] -= 5;
        if (obsX[i] + obsWidth < pic_x && obsX[i] + obsWidth + 5 >= pic_x) {
    score += 10;  // Increase score when player passes an obstacle
}


        // Recycle obstacle if it goes off-screen
        if (obsX[i] + obsWidth < 0) {
            obsX[i] = 800 + rand() % 500 + 200;  // random space between 200 to 700
        }

        // Check for collision
        if (checkCollision(pic_x, pic_y, obsX[i], obsY, obsWidth, obsHeight)) {
            printf("Collision detected!\n");
            gameOver = true;
        }
    }
}

void iKeyboard(unsigned char key)
{
    if ((key == ' ' || key == ' ') && !gameOver) {
        if (!jump) {
            jump = 1;
            jumpSpeed = jumpHeight;
        }
    }
    else if (key == 'r' || key == 'R') {  // Restart game
        gameOver = false;
        pic_x = 320;
        pic_y = groundY;

        // Reset obstacles positions spaced farther apart
        int startX = 1000;   // Start farther than before
        for (int i = 0; i < NUM_OBSTACLES; i++) {
            obsX[i] = startX + i * 500;  // 500 pixels apart
        }
    }
}

void iSpecialKeyboard(unsigned char key)
{
    if (gameOver) return;  // no movement if game over

    if (key == GLUT_KEY_RIGHT) {
        pic_x += moveSpeed;
        if (pic_x > 800 - 50) pic_x = 800 - 50;
    }
    else if (key == GLUT_KEY_LEFT) {
        pic_x -= moveSpeed;
        if (pic_x < 0) pic_x = 0;
    }
    else if (key == GLUT_KEY_END) {
        exit(0);
    }
}

void iMouseMove(int mx, int my) {}
void iMouseDrag(int mx, int my) {}
void iMouse(int button, int state, int mx, int my) {}
void iMouseWheel(int dir, int mx, int my) {}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);

    populate_run_images();
    iSetTimer(100, update_run);
    iSetTimer(20, iUpdate);
    iInitialize(800, 500, "Super Mario");

    return 0;
}
