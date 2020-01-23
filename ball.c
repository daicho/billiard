#include <math.h>

#include <GL/glut.h>
#include <GL/glpng.h>

#include "ball.h"
#include "vector.h"

// 角度を変換
#define rad(deg) (rad * M_PI / 180.0)
#define degree(rad) (rad * 180.0 / M_PI)

// ball構造体を初期化
void initBall(struct ball *ball, int num, double px, double py, double r) {
    ball->num = num;
    ball->exist = 1;
    ball->r = r;
    ball->p = vector(px, py);
    ball->v = ZERO;
    ball->dir = vector(1, 0);
    ball->angle = 0;
}

// 移動
void moveBall(struct ball *ball) {
    // 移動
    ball->p = add(ball->p, ball->v);

    // 回転
    ball->angle = fmod(ball->angle + mag(ball->v) / (2 * ball->r), 2 * M_PI);

    if (!isZero(ball->v))
        ball->dir = normal(ball->v);

    // 摩擦
    if (mag(ball->v) <= 0.001)
        ball->v = ZERO;
    else
        ball->v = mult(ball->v, 1 - FRICTION / mag(ball->v));
}

// ボールを描画
void drawBall(struct ball ball) {
    GLUquadricObj* sphere = gluNewQuadric();
    gluQuadricDrawStyle(sphere, GLU_FILL);
    gluQuadricNormals(sphere, GLU_SMOOTH);
    gluQuadricTexture(sphere, GL_TRUE);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, ball.image);

    glPushMatrix();
    glTranslated(ball.p.x, ball.p.y, ball.r);
    glRotated(90, 0, 1, 0);
    glRotated(90, 1, 0, 0);
    glRotated(degree(ball.angle), 0, -ball.dir.y, -ball.dir.x);
    gluSphere(sphere, ball.r, 32, 32);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

// ボール同士の衝突判定
void collideBall(struct ball *ballA, struct ball *ballB) {
    if (dist(ballA->p, ballB->p) < ballA->r + ballB->r) {
        struct vector dir_p, dir_v, temp;

        // ボールの重なりを修正
        dir_p = sub(ballA->p, ballB->p);
        temp = mult(normal(dir_p), (ballA->r + ballB->r - mag(dir_p)) / 2);
        ballA->p = add(ballA->p, temp);
        ballB->p = sub(ballB->p, temp);

        // 2つのボールの相対位置と相対速度
        dir_p = sub(ballA->p, ballB->p);
        dir_v = sub(ballA->v, ballB->v);

        // 衝突後の速度を決定
        temp = mult(normal(dir_p), -inner(normal(dir_p), dir_v));
        ballA->v = mult(add(ballA->v, temp), BALL_LOSS);
        ballB->v = mult(sub(ballB->v, temp), BALL_LOSS);
    }
}
