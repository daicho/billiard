#ifndef TABLE_H
#define TABLE_H

#include <GL/glut.h>
#include <GL/glpng.h>

#include "ball.h"
#include "vector.h"

// 台
struct table {
    struct vector size; // 大きさ
    double pocket_r;    // ポケットの半径
    double wall_loss;   // 壁衝突時の速度損失
    GLuint image;       // 画像
};

void reflectTable(struct table, struct ball *);
int pocketTouching(struct table, struct ball);
void pocketIn(struct table, struct ball *);

#endif
