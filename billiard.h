#ifndef BILLIARD_H
#define BILLIARD_H

#include <GL/glut.h>
#include <GL/glpng.h>

#include "vector.h"

#define BALL_NUM 10

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

// テーブル
struct table {
    int pocket_num;         // ポケットの数
    struct vector *pockets; // ポケットの座標
    double pocket_r;        // ポケットの半径
    void (*collide)(void);  // 衝突判定を行う関数
    GLuint image;           // 画像
};

// キュー
struct cue {
    int exist;    // 存在しているか
    double angle; // 角度
    GLuint image; // 画像
};

void Display(void);
void Reshape(int, int);
void Timer(int);
void Mouse(int, int, int, int);
void PassiveMotion(int, int);

void init(void);
void update(void);
void initBall(struct ball *, int, double, double, double);
void drawBall(struct ball);

void collideSquare(void);
void collideCircle(void);
void pocket(void);

extern struct ball balls[BALL_NUM];
extern struct vector pockets[6];
extern struct table table;
extern struct cue cue;

#endif
