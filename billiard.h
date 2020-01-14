#ifndef BILLIARD_H
#define BILLIARD_H

#define BALL_NUM 9

// 二次元ベクトル
struct vector {
    double x;
    double y;
};

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
void drawCircle(double, double, double);
void drawBall(struct ball);

extern struct ball cueBall;
extern struct ball balls[BALL_NUM];

#endif
