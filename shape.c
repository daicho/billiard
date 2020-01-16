#include <math.h>

#include <GL/glut.h>
#include <GL/glpng.h>

#include "shape.h"

// 正円を描画
void drawCircle(double x, double y, double r) {
    int i;
    int h = 30;

    glBegin(GL_LINE_LOOP);

    for (i = 0; i < h; i++) {
        double theta = 2 * M_PI * i / h;
        glVertex2d(x + r * sin(theta), y + r * cos(theta));
    }

    glEnd();
}

// 画像を表示
// num : 画像の番号
// x, y : 座標
// w, h : サイズ
// pngInfo : 画像情報
void putSprite(GLuint num, double x, double y, double w, double h, pngInfo *info) {
    float color[4];

    // 現在の色を保存
    glGetFloatv(GL_CURRENT_COLOR, color);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, num);
    glColor4ub(255, 255, 255, 255);

    glBegin(GL_QUADS);

    glTexCoord2i(0, 0);
    glVertex2d(x, y + h);

    glTexCoord2i(0, 1);
    glVertex2d(x, y);

    glTexCoord2i(1, 1);
    glVertex2d(x + w, y);

    glTexCoord2i(1, 0);
    glVertex2d(x + w, y + h);

    glEnd();
    glColor3fv(color);
    glDisable(GL_TEXTURE_2D);
}
