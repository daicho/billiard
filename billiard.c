#include <stdio.h>
#include <math.h>
#include <time.h>

#include <GL/glut.h>
#include <GL/glpng.h>

#include "billiard.h"
#include "vector.h"
#include "shape.h"

#define FPS      60           // フレームレート
#define ASPECT   (16.0 / 9.0) // アスペクト比(幅/高さ)
#define FRICTION 0.0002       // 摩擦
#define BALL_R   0.05         // ボールの半径

// ボール
struct ball balls[BALL_NUM];

int main(int argc, char *argv[]) {
    // 初期化
    glutInit(&argc, argv);
    glutInitWindowSize(640, 360);
    glutCreateWindow("Billiard");
    glutInitDisplayMode(GLUT_RGBA);
    glClearColor(1.0, 1.0, 1.0, 1.0);

    // 混合処理を有効化
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // 線のアンチエイリアスを有効化
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    // 画像読み込み
    //dial_img = pngBind("dial.png", PNG_NOMIPMAP, PNG_ALPHA, &dial_info, GL_CLAMP, GL_NEAREST, GL_NEAREST);

    // コールバック関数登録
    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutTimerFunc(1000.0 / FPS + 0.5, Timer, 0);
    glutMouseFunc(Mouse);
    glutPassiveMotionFunc(PassiveMotion);
    glutMotionFunc(Motion);

    // メインループ開始
    init();
    glutMainLoop();

    return 0;
}

// 画面描画
void Display(void) {
    int i;

    glClear(GL_COLOR_BUFFER_BIT);

    glColor3ub(0, 0, 0);

    for (i = 0; i < BALL_NUM; i++) {
        drawBall(balls[i]);
    }

    glFlush();
}

// ウィンドウサイズ変更
void Reshape(int w, int h) {
    // ウィンドウの縦横比
    double ratio = (double)w / h;

    // 座標系再設定
    glViewport(0, 0, w, h);
    glLoadIdentity();

    if (ratio > ASPECT)
        gluOrtho2D(-ratio, ratio, -1, 1);
    else
        gluOrtho2D(-ASPECT, ASPECT, -ASPECT / ratio, ASPECT / ratio);
}

// タイマー
void Timer(int value) {
    // 次のタイマーを登録
    glutTimerFunc(1000.0 / FPS + 0.5, Timer, 0);
    update();
    Display();
}

// マウスクリック
void Mouse(int b, int s, int x, int y) {
    struct vector point;
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);

    // ウィンドウの縦横比
    double ratio = (double)w / h;

    if (ratio > ASPECT)
        set(&point, ((double)x / w - 0.5) * 2 * ratio, ((double)y / h - 0.5) * -2);
    else
        set(&point, ((double)x / w - 0.5) * 2 * ASPECT, ((double)y / h - 0.5) * 2 * ASPECT / -ratio);

    if (b == GLUT_LEFT_BUTTON && s == GLUT_DOWN)
        set(&balls[0].v, (point.x - balls[0].p.x) / 20, (point.y - balls[0].p.y) / 20);
}

// マウス移動
void PassiveMotion(int x, int y) {
}

// マウスドラッグ
void Motion(int x, int y) {
}

// 初期化
void init(void) {
    struct vector p1 = {0.5, 0}; // 1番玉の位置
    double r = BALL_R + 0.001;   // ボールの間隔

    initBall(&balls[0], 0, 0, 0, BALL_R);
    initBall(&balls[1], 1, p1.x, p1.y, BALL_R);
    initBall(&balls[2], 2, p1.x + r * 2 * sqrt(3), p1.y - r * 2, BALL_R);
    initBall(&balls[3], 3, p1.x + r * 4 * sqrt(3), p1.y, BALL_R);
    initBall(&balls[4], 4, p1.x + r * 2 * sqrt(3), p1.y + r * 2, BALL_R);
    initBall(&balls[5], 5, p1.x + r * sqrt(3), p1.y - r, BALL_R);
    initBall(&balls[6], 6, p1.x + r * sqrt(3), p1.y + r, BALL_R);
    initBall(&balls[7], 7, p1.x + r * 3 * sqrt(3), p1.y - r, BALL_R);
    initBall(&balls[8], 8, p1.x + r * 3 * sqrt(3), p1.y + r, BALL_R);
    initBall(&balls[9], 9, p1.x + r * 2 * sqrt(3), p1.y, BALL_R);
}

// 更新
void update(void) {
    int i, j;

    for (i = 0; i < BALL_NUM; i++) {
        add(&balls[i].p, &balls[i].v);

        if (mag(balls[i].v) <= 0.0001) {
            set(&balls[i].v, 0, 0);
        } else {
            mult(&balls[i].v, 1 - FRICTION / mag(balls[i].v));
            //balls[i].v.y -= FRICTION * balls[i].v.y / mag(balls[i].v);
        }
    }

    for (i = 0; i < BALL_NUM; i++) {
        for (j = i + 1; j < BALL_NUM; j++) {
            if (dist(balls[i].p, balls[i].p) < BALL_R * 2) {
                struct vector dir_p, dir_v, temp;
                double dist;

                dir_p = balls[i].p;
                sub(&dir_p, &balls[j].p);
                dist = mag(dir_p);
                divi(&dir_p, dist);

                temp = dir_p;
                mult(&temp, BALL_R - dist / 2);

                add(&balls[i].p, &temp);
                sub(&balls[j].p, &temp);

                dir_p = balls[i].p;
                sub(&dir_p, &balls[j].p);
                divi(&dir_p, BALL_R * 2);

                dir_v = balls[i].v;
                sub(&dir_v, &balls[j].v);

                temp = dir_p;
                mult(&temp, -inner(dir_p, dir_v));

                add(&balls[i].v, &temp);
                sub(&balls[j].v, &temp);
            }
        }
    }

    for (i = 0; i < BALL_NUM; i++) {
        if (balls[i].p.x > 1) {
            balls[i].p.x = 1;
            balls[i].v.x *= -1;
        }

        if (balls[i].p.x < -1) {
            balls[i].p.x = -1;
            balls[i].v.x *= -1;
        }

        if (balls[i].p.y > 1) {
            balls[i].p.y = 1;
            balls[i].v.y *= -1;
        }

        if (balls[i].p.y < -1) {
            balls[i].p.y = -1;
            balls[i].v.y *= -1;
        }
    }
}

// ball構造体を初期化
void initBall(struct ball *ball, int num, double px, double py, double r) {
    ball->num = num;
    set(&ball->p, px, py);
    set(&ball->v, 0, 0);
    ball->r = r;
}

// ボールを描画
void drawBall(struct ball ball) {
    drawCircle(ball.p.x, ball.p.y, ball.r);
}
