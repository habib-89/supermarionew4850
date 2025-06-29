#include "iGraphics.h"

#define TILE_SIZE 40
#define ROWS 5
#define COLS 30

int tileMap[ROWS][COLS] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

int pic_x = 100;
int pic_y = TILE_SIZE * 5;
int jump = 0;
int jumpSpeed = 0;

void iDraw() {
    iClear();

    // Draw tile map
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if (tileMap[row][col] == 1) {
                iSetColor(139, 69, 19);
                iFilledRectangle(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE);
            }
        }
    }

    // Draw player (Mario)
    iSetColor(255, 0, 0);
    iFilledRectangle(pic_x, pic_y, 30, 40);

    // Draw HUD
    iSetColor(255, 255, 255);
    char info[100];
    sprintf(info, "SCORE: %d  COINS: %d  WORLD: 1-1  LIVES: %d", 0, 0, 3);
    iText(10, 470, info, GLUT_BITMAP_HELVETICA_18);
}

void iUpdate() {
    // Gravity and jump
    int colUnder = (pic_x + 15) / TILE_SIZE;
    int rowUnder = (pic_y) / TILE_SIZE;

    if (tileMap[rowUnder][colUnder] == 1) {
        pic_y = rowUnder * TILE_SIZE + TILE_SIZE;
        jump = 0;
        jumpSpeed = 0;
    } else {
        jump = 1;
    }

    if (jump) {
        pic_y -= 5;
    } else {
        if (jumpSpeed > 0) {
            pic_y += jumpSpeed;
            jumpSpeed--;
        }
    }
}

void iKeyboard(unsigned char key) {
    if (key == ' ') {
        if (!jump) {
            jump = 1;
            jumpSpeed = 10;
        }
    }
}

void iSpecialKeyboard(unsigned char key) {
    if (key == GLUT_KEY_RIGHT) {
        pic_x += 10;
    }
    if (key == GLUT_KEY_LEFT) {
        pic_x -= 10;
    }
}

int main() {
    iSetTimer(20, iUpdate);
    iInitialize(1200, 500, "Mario Tile Game");
    return 0;
}
