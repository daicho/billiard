#ifndef BILLIARD_H
#define BILLIARD_H

#include "vector.h"

#define BALL_NUM 10

// ボール
struct ball {
    int num;         // 番号 (0:白玉)
    struct vector p; // 位置
    struct vector v; // 速度
    double r;        // 半径
};

void Display(void);
void Reshape(int, int);
void Timer(int);
void Mouse(int, int, int, int);
void PassiveMotion(int, int);
void Motion(int, int);

void init(void);
void update(void);
void initBall(struct ball *, int, double, double, double);
void drawBall(struct ball);

extern struct ball balls[BALL_NUM];

#endif
