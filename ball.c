#include <math.h>

#include <GL/glut.h>
#include <GL/glpng.h>

#include "ball.h"
#include "vector.h"

// 角度を変換
#define radian(deg) (rad * M_PI / 180.0)
#define degree(rad) (rad * 180.0 / M_PI)

// ball構造体を初期化
void initBall(struct ball *ball, int num, struct vector p, double r) {
    ball->num = num;
    ball->exist = 1;
    ball->r = r;
    ball->p = p;
    ball->v = ZERO;
    ball->dir = vector(1, 0);
    ball->angle = 0;
}

// 移動
void moveBall(struct ball *ball) {
    ball->p = add(ball->p, ball->v);
    ball->angle = fmod(ball->angle + mag(ball->v) / ball->r, 2 * M_PI);
    if (!isZero(ball->v)) ball->dir = normal(ball->v);

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

// ボールが衝突しているか
int ballColliding(struct ball ballA, struct ball ballB) {
    return dist(ballA.p, ballB.p) < ballA.r + ballB.r;
}

// ボール同士の反射
void reflectBall(struct ball *ballA, struct ball *ballB, int break_shot) {
    if (ballColliding(*ballA, *ballB)) {
        struct vector p_ab, v_ab, temp;
        double a, b, c, D, t;

        // ボールの重なりを修正
        p_ab = sub(ballA->p, ballB->p);
        v_ab = sub(ballA->v, ballB->v);
        a = pow(mag(v_ab), 2);
        b = inner(p_ab, v_ab);
        c = pow(mag(p_ab), 2) - pow(ballA->r + ballB->r, 2);
        D = pow(b, 2) - a * c;

        if (D > 0 && !break_shot) {
            t = (-b - sqrt(D)) / a;
            ballA->p = add(ballA->p, mult(ballA->v, t));
            ballB->p = add(ballB->p, mult(ballB->v, t));
        } else {
            temp = mult(normal(p_ab), (ballA->r + ballB->r - mag(p_ab)) / 2);
            ballA->p = add(ballA->p, temp);
            ballB->p = sub(ballB->p, temp);
        }

        // 2つのボールの相対位置と相対速度
        p_ab = sub(ballA->p, ballB->p);
        v_ab = sub(ballA->v, ballB->v);

        // 衝突後の速度を決定
        temp = mult(normal(p_ab), -inner(normal(p_ab), v_ab));
        ballA->v = mult(add(ballA->v, temp), BALL_LOSS);
        ballB->v = mult(sub(ballB->v, temp), BALL_LOSS);
    }
}
