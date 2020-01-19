#ifndef BILLIARD_H
#define BILLIARD_H

#include <GL/glut.h>
#include <GL/glpng.h>

#include "vector.h"

#define BALL_NUM 10

// ボール
struct ball {
    int num;           // 番号 (0:白玉)
    int exist;         // 存在しているか
    struct vector p;   // 位置
    struct vector v;   // 速度
    double r;          // 半径
    GLfloat color[4]; // 色
};

// テーブル
struct table {
    int pocket_num;
    struct vector *pockets;
    double pocket_r;
    void (*collide)(void);
    GLuint img;
};

void Display(void);
void Reshape(int, int);
void Timer(int);
void Mouse(int, int, int, int);
void PassiveMotion(int, int);
void Motion(int, int);

void init(void);
void update(void);
void initBall(struct ball *, int, double, double, double, GLfloat, GLfloat, GLfloat, GLfloat);
void drawBall(struct ball);

void collideSquare(void);
void collideCircle(void);
void pocket(void);

extern struct ball balls[BALL_NUM];
extern struct vector pockets[6];
extern struct table table;

#endif
