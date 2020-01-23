#ifndef BILLIARD_H
#define BILLIARD_H

#include <GL/glut.h>
#include <GL/glpng.h>

#include "ball.h"
#include "vector.h"

#define WALL_LOSS 0.8 // 壁衝突時の速度損失
#define BALL_NUM 10   // ボールの数

// テーブル
struct table {
    int pocket_num;         // ポケットの数
    struct vector *pockets; // ポケットの座標
    double pocket_r;        // ポケットの半径
    void (*collide)(struct ball *); // 衝突判定を行う関数
    GLuint image;           // 画像
};

// キュー
struct cue {
    int exist;       // 存在しているか
    struct vector p; // 位置
    double angle;    // 角度
    GLuint image;    // 画像
};

void init(void);
void update(void);
void initBall(struct ball *, int, double, double, double);
void drawBall(struct ball);

void collideSquare(struct ball *);
void collideCircle(struct ball *);
void pocket(struct table, struct ball *);

struct vector convertPoint(int, int);

void Display(void);
void Reshape(int, int);
void Timer(int);
void Mouse(int, int, int, int);
void PassiveMotion(int, int);

extern struct ball balls[BALL_NUM];
extern struct vector pockets[6];
extern struct table table;
extern struct cue cue;
extern int pulling;

#endif
