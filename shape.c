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
// x, y, z : 座標
// w, h : サイズ
void putSprite(GLuint num, double x, double y, double z, double w, double h) {
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, num);
    glColor4d(1.0, 1.0, 1.0, 1.0);

    glBegin(GL_QUADS);

    glTexCoord2d(0, 0);
    glVertex3d(x - w / 2, y + h / 2, z);

    glTexCoord2d(0, 1);
    glVertex3d(x - w / 2, y - h / 2, z);

    glTexCoord2d(1, 1);
    glVertex3d(x + w / 2, y - h / 2, z);

    glTexCoord2d(1, 0);
    glVertex3d(x + w / 2, y + h / 2, z);

    glEnd();
    glDisable(GL_TEXTURE_2D);
}
