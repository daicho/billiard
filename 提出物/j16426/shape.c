// 16426 福澤大地

#include <math.h>

#include <GL/glut.h>
#include <GL/glpng.h>

#include "shape.h"

// 正円を描画
void drawCircle(double x, double y, double r, int h) {
    int i;

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
// a : 不透明度
void putSprite(GLuint num, double x, double y, double w, double h, double a) {
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, num);
    glColor4d(1.0, 1.0, 1.0, a);

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
    glEnable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
}
