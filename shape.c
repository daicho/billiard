#include <math.h>

#include <GL/glut.h>
#include <GL/glpng.h>

#include "shape.h"

// 正円を描画
void drawCircle(double x, double y, double r) {
    int i;
    int h = 50;

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
void putSprite(GLuint num, double x, double y, double w, double h) {
    GLdouble color[4];

    // 現在の色を保存
    glGetDoublev(GL_CURRENT_COLOR, color);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, num);
    glColor4d(1.0, 1.0, 1.0, 1.0);

    glBegin(GL_QUADS);

    glTexCoord2d(0, 0);
    glVertex2d(x - w / 2, y + h / 2);

    glTexCoord2d(0, 1);
    glVertex2d(x - w / 2, y - h / 2);

    glTexCoord2d(1, 1);
    glVertex2d(x + w / 2, y - h / 2);

    glTexCoord2d(1, 0);
    glVertex2d(x + w / 2, y + h / 2);

    glEnd();
    glColor3dv(color);
    glDisable(GL_TEXTURE_2D);
}
