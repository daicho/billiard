#ifndef BILLIARD_H
#define BILLIARD_H

#include <GL/glut.h>
#include <GL/glpng.h>

#include "ball.h"
#include "vector.h"

#define BALL_NUM  10  // ボールの数

// テーブル
struct table {
    struct vector size; // 大きさ
    double pocket_r;    // ポケットの半径
    double wall_loss;   // 壁衝突時の速度損失
    GLuint image;       // 画像
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
int movingBall(void);

void collideTable(struct table, struct ball *);
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
extern struct vector mouse;
extern int pulling;
extern double power;

#endif
