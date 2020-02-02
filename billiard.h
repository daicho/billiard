#ifndef BILLIARD_H
#define BILLIARD_H

#include <GL/glut.h>
#include <GL/glpng.h>

#include "ball.h"
#include "vector.h"

#define BALL_NUM    10  // ボールの数
#define TURN_TIME   120 // ターン表示の時間
#define RESULT_TIME 240 // リザルト表示の時間

// テーブル
struct table {
    struct vector size; // 大きさ
    double pocket_r;    // ポケットの半径
    double wall_loss;   // 壁衝突時の速度損失
    GLuint image;       // 画像
};

// キュー
struct cue {
    struct vector p; // 位置
    double angle;    // 角度
    double power;    // 力
    GLuint image;    // 画像
};

// シーン
enum scene {
    Title, // タイトル
    Game,  // ゲーム画面
};

// 状態
enum status {
    Stop,  // 静止中
    Pull,  // キューを引いてる
    Put,   // 手玉を配置
    Move,  // 動いている
    Result // 結果
};

// ターン
enum turn {
    Player1,
    Player2,
    Player,
    CPU
};

void init(void);
void update(void);

void reflectTable(struct table, struct ball *);
int pocketTouching(struct table, struct ball);
void pocketIn(struct table, struct ball *);
int ballMoving(void);
int canPut(void);

struct vector convertPoint(int, int);

void Display(void);
void Reshape(int, int);
void Timer(int);
void Mouse(int, int, int, int);
void PassiveMotion(int, int);

extern struct ball prev_balls[BALL_NUM];
extern struct ball balls[BALL_NUM];
extern struct table table;
extern struct cue cue;

extern GLuint title_image;
extern GLuint vshuman_images[2];
extern GLuint hscpu_images[2];
extern GLuint invalid_image;
extern GLuint highlight_image;
extern GLuint turn_images[4];
extern GLuint win_images[4];

extern struct vector mouse;
extern enum scene scene;
extern enum status status;
extern enum turn turn;
extern int turn_left;
extern int win_left;
extern int break_shot;
extern int next;
extern int first_touch;

#endif
