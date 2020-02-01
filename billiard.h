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
    double power;    // 力
    GLuint image;    // 画像
};

// 状態
enum status {
    Stop, // 静止中
    Pull, // キューを引いてる
    Put,  // 手玉を配置
    Move  // 動いている
};

void init(void);
void update(void);
int ballMoving(void);

void collideTable(struct table, struct ball *);
void pocketIn(struct table, struct ball *);

struct vector convertPoint(int, int);

void Display(void);
void Reshape(int, int);
void Timer(int);
void Mouse(int, int, int, int);
void PassiveMotion(int, int);

extern struct ball balls[BALL_NUM];
extern struct table table;
extern struct cue cue;

extern enum status status;
extern int break_shot;
extern int next;
extern int first_touch;

#endif
