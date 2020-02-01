#ifndef BALL_H
#define BALL_H

#include <GL/glut.h>
#include <GL/glpng.h>

#include "vector.h"

#define FRICTION  0.0002 // 摩擦
#define BALL_LOSS 0.95   // ボール衝突時の速度損失

// ボール
struct ball {
    int exist;         // 存在しているか
    int num;           // 番号 (0:白玉)
    double r;          // 半径
    struct vector p;   // 位置
    struct vector v;   // 速度
    struct vector dir; // 進行方向
    double angle;      // 角度
    GLuint image;      // 画像
};

void initBall(struct ball *, int, double, double, double);
void moveBall(struct ball *);
void drawBall(struct ball);
int ballColliding(struct ball, struct ball);
void reflectBall(struct ball *, struct ball *, int);

#endif
