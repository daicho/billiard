#include <stdio.h>
#include <math.h>
#include <time.h>

#include <GL/glut.h>
#include <GL/glpng.h>

#include "billiard.h"
#include "ball.h"
#include "vector.h"
#include "shape.h"

#define FPS      60    // フレームレート
#define ASPECT   2     // アスペクト比 (幅/高さ)
#define BALL_R   0.04  // ボールの半径
#define TABLE_W  1.75  // テーブルの幅
#define TABLE_H  0.875 // テーブルの高さ
#define CUE_W    1.024 // キューの幅
#define CUE_H    0.032 // キューの高さ

// 角度を変換
#define radian(deg) (deg * M_PI / 180.0)
#define degree(rad) (rad * 180.0 / M_PI)

// ボール
struct ball balls[BALL_NUM];

// テーブル
struct vector pockets[6] = {
    {-TABLE_W, TABLE_H},
    {0,        TABLE_H},
    {TABLE_W,  TABLE_H},
    {-TABLE_W, -TABLE_H},
    {0,        -TABLE_H},
    {TABLE_W,  -TABLE_H}
};

struct table table = {6, pockets, 0.0896, collideSquare};
struct cue cue = {1, {0, 0}, 0};
struct vector mouse = {0, 0};
int pulling = 0;
double power = 0;

int main(int argc, char *argv[]) {
    int i;

    // 初期化
    glutInit(&argc, argv);
    glutInitWindowSize(720, 360);
    glutCreateWindow("Billiard");
    glutInitDisplayMode(GLUT_RGBA);
    glClearColor(1.0, 1.0, 1.0, 1.0);

    // 混合処理を有効化
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 線のアンチエイリアスを有効化
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    // 陰影を有効化
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_CULL_FACE);

    // 画像読み込み
    table.image = pngBind("images/square.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_NEAREST, GL_NEAREST);
    cue.image = pngBind("images/cue.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_NEAREST, GL_NEAREST);

    for (i = 0; i < BALL_NUM; i++) {
        char fileName[32];
        sprintf(fileName, "images/%d.png", i);
        balls[i].image = pngBind(fileName, PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_NEAREST, GL_NEAREST);
    }

    // コールバック関数登録
    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutTimerFunc(1000.0 / FPS + 0.5, Timer, 0);
    glutMouseFunc(Mouse);
    glutPassiveMotionFunc(PassiveMotion);

    // メインループ開始
    init();
    glutMainLoop();

    return 0;
}

// 初期化
void init(void) {
    struct vector p1 = {TABLE_W / 2, 0}; // 1番玉の位置
    double r = BALL_R + 0.0001; // ボールの間隔

    // ボールを配置
    initBall(&balls[0], 0, -TABLE_W / 2, 0, BALL_R);
    initBall(&balls[1], 1, p1.x, p1.y, BALL_R);
    initBall(&balls[2], 2, p1.x + r * 2 * sqrt(3), p1.y - r * 2, BALL_R);
    initBall(&balls[3], 3, p1.x + r * 4 * sqrt(3), p1.y, BALL_R);
    initBall(&balls[4], 4, p1.x + r * 2 * sqrt(3), p1.y + r * 2, BALL_R);
    initBall(&balls[5], 5, p1.x + r * sqrt(3), p1.y - r, BALL_R);
    initBall(&balls[6], 6, p1.x + r * sqrt(3), p1.y + r, BALL_R);
    initBall(&balls[7], 7, p1.x + r * 3 * sqrt(3), p1.y - r, BALL_R);
    initBall(&balls[8], 8, p1.x + r * 3 * sqrt(3), p1.y + r, BALL_R);
    initBall(&balls[9], 9, p1.x + r * 2 * sqrt(3), p1.y, BALL_R);

    // キューを配置
    cue.p = balls[0].p;
}

// 更新
void update(void) {
    int i, j;
    int colliding;

    // 移動
    for (i = 0; i < BALL_NUM; i++) {
        if (balls[i].exist)
            moveBall(&balls[i]);
    }

    // ボール同士の衝突
    for (i = 0; i < BALL_NUM; i++) {
        if (!balls[i].exist) continue;

        for (j = i + 1; j < BALL_NUM; j++) {
            if (balls[j].exist)
                collideBall(&balls[i], &balls[j]);
        }

        // テーブルとの衝突
        table.collide(&balls[i]);
    }

    // ポケット判定
    for (i = 0; i < BALL_NUM; i++) {
        if (balls[i].exist)
            pocket(table, &balls[i]);
    }

    // キューを引く
    if (pulling) {
        power += 0.1;
        if (power > 0.15)
            power = 0.15;
    }
}

// ボールが動いているか
int movingBall(void) {
    int i;

    for (i = 0; i < BALL_NUM; i++) {
        if (!isZero(balls[i].v))
            return 1;
    }

    return 0;
}

// 四角いテーブルの衝突判定
void collideSquare(struct ball *ball) {
    if (ball->p.x > TABLE_W - ball->r) {
        ball->p.x = TABLE_W - ball->r;
        ball->v.x *= -WALL_LOSS;
    }

    if (ball->p.x < -TABLE_W + ball->r) {
        ball->p.x = -TABLE_W + ball->r;
        ball->v.x *= -WALL_LOSS;
    }

    if (ball->p.y > TABLE_H - balls->r) {
        ball->p.y = TABLE_H - balls->r;
        ball->v.y *= -WALL_LOSS;
    }

    if (ball->p.y < -TABLE_H + balls->r) {
        ball->p.y = -TABLE_H + balls->r;
        ball->v.y *= -WALL_LOSS;
    }
}

// 丸いテーブルの衝突判定
void collideCircle(struct ball *ball) {
    if (mag(ball->p) > 1 - balls->r) {
        ball->p = mult(normal(ball->p), 1 - ball->r);
        ball->v = mult(rotate(ball->v, M_PI - (angle(ball->v) - angle(ball->p)) * 2), WALL_LOSS);
    }
}

// ポケットに入ったか
void pocket(struct table table, struct ball *ball) {
    int i;

    for (i = 0; i < table.pocket_num; i++) {
        if (dist(ball->p, table.pockets[i]) < table.pocket_r) {
            if (ball->num == 0) {
                ball->p = vector(-TABLE_W / 2, 0);
            } else {
                ball->exist = 0;
            }

            ball->v = ZERO;

            break;
        }
    }
}

// 描画時の座標に変換
struct vector convertPoint(int x, int y) {
    struct vector point;
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);

    // ウィンドウの縦横比
    double ratio = (double)w / h;

    // 座標を変換
    if (ratio > ASPECT)
        point = vector(((double)x / w - 0.5) * 2 * ratio, ((double)y / h - 0.5) * -2);
    else
        point = vector(((double)x / w - 0.5) * 2 * ASPECT, ((double)y / h - 0.5) * 2 * ASPECT / -ratio);

    return point;
}

// 画面描画
void Display(void) {
    int i;
    int target = 0;
    double predict_min = 0;

    GLfloat lightPos[2][4] = {
        {-1.0, 0.0, 10.0, 1.0},
        {1.0, 0.0, 10.0, 1.0}
    };

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 光源を設定
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos[0]);
    glLightfv(GL_LIGHT1, GL_POSITION, lightPos[1]);

    // テーブル
    putSprite(table.image, 0, 0, 0, ASPECT * 2, 2);

    // 予測線
    if (!movingBall()) {
        for (i = 1; i < BALL_NUM; i++) {
            double touch;
            double predict;

            if (!balls[i].exist || !cue.exist) continue;
            if (cos(angle(sub(balls[i].p, balls[0].p)) - cue.angle) < 0) continue;

            // 手玉の軌道上との距離を算出
            touch = fabs(tan(cue.angle) * (balls[i].p.x - balls[0].p.x) - balls[i].p.y + balls[0].p.y) / mag(vector(tan(cue.angle), 1));

            if (touch < balls[0].r + balls[i].r) {
                // 衝突予想地点を算出
                predict = dist(balls[0].p, balls[i].p) * cos(angle(sub(balls[i].p, balls[0].p)) - cue.angle) - sqrt(pow(balls[0].r + balls[i].r, 2) - pow(touch, 2));

                if (!target || predict < predict_min) {
                    target = i;
                    predict_min = predict;
                }
            }
        }

        // 予測線を描画
        glDisable(GL_LIGHTING);
        glPushMatrix();
        glTranslated(balls[0].p.x, balls[0].p.y, 0);
        glRotated(degree(cue.angle), 0, 0, 1);
        glBegin(GL_LINES);
        glColor3d(0.0, 0.0, 0.0);
        glVertex2d(0, 0);
        glVertex2d(predict_min - balls[0].r, 0);
        glEnd();

        drawCircle(predict_min, 0, balls[0].r);
        glPopMatrix();
        glEnable(GL_LIGHTING);
    }

    // ボール
    for (i = 0; i < BALL_NUM; i++) {
        if (balls[i].exist)
            drawBall(balls[i]);
    }

    // キュー
    if (cue.exist) {
        glPushMatrix();
        glTranslated(cue.p.x, cue.p.y, 0);
        glRotated(degree(cue.angle) + 180, 0, 0, 1);
        putSprite(cue.image, balls[0].r + CUE_W / 2 + power * 2, 0, balls[0].r * 2, CUE_W, CUE_H);
        glPopMatrix();
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
    glutPostRedisplay();
}

// マウスクリック
void Mouse(int b, int s, int x, int y) {
    mouse = convertPoint(x, y);

    if (b == GLUT_LEFT_BUTTON) {
        if (s == GLUT_DOWN && !movingBall()) {
            pulling = 1;
            cue.angle = angle(sub(mouse, balls[0].p));
            cue.p = balls[0].p;
        }

        if (s == GLUT_UP && pulling) {
            pulling = 0;
            balls[0].v = mult(vector(cos(cue.angle), sin(cue.angle)), power);
            power = 0;
        }
    }
}

// マウス移動
void PassiveMotion(int x, int y) {
    mouse = convertPoint(x, y); 

    if (!movingBall()) {
        cue.angle = angle(sub(mouse, balls[0].p));
        cue.p = balls[0].p;
    }
}
