#include <stdio.h>
#include <math.h>
#include <time.h>

#include <GL/glut.h>
#include <GL/glpng.h>

#include "billiard.h"
#include "vector.h"
#include "shape.h"

#define FPS       60     // フレームレート
#define ASPECT    2      // アスペクト比 (幅/高さ)
#define BALL_R    0.0393 // ボールの半径
#define FRICTION  0.0003 // 摩擦
#define WALL_LOSS 0.8    // 壁衝突時の速度損失
#define BALL_LOSS 0.95   // ボール衝突時の速度損失
#define TABLE_W   1.75   // テーブルの幅
#define TABLE_H   0.875  // テーブルの高さ
#define CUE_W     1.024  // キューの幅
#define CUE_H     0.032  // キューの高さ

// 角度を変換
#define rad(deg) (rad * M_PI / 180.0)
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
struct cue cue = {1, 0};

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
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

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

// 画面描画
void Display(void) {
    int i;
    GLfloat lightPos[2][4] = {
        {-1.0, 0.0, 10.0, 1.0},
        {1.0, 0.0, 10.0, 1.0}
    };

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos[0]);
    glLightfv(GL_LIGHT1, GL_POSITION, lightPos[1]);

    putSprite(table.image, 0, 0, 0, ASPECT * 2, 2);

    if (cue.exist) {
        glPushMatrix();
        glTranslated(balls[0].p.x, balls[0].p.y, 0);
        glRotated(degree(cue.angle), 0, 0, 1);
        putSprite(cue.image, balls[0].r + CUE_W / 2, 0, balls[0].r * 3, CUE_W, CUE_H);
        glPopMatrix();
    }

    for (i = 0; i < BALL_NUM; i++) {
        if (balls[i].exist)
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
    int i;
    int moving = 0;
    struct vector point;
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);

    // ウィンドウの縦横比
    double ratio = (double)w / h;

    // マウスの座標を描画時の座標系に変換
    if (ratio > ASPECT)
        point = vector(((double)x / w - 0.5) * 2 * ratio, ((double)y / h - 0.5) * -2);
    else
        point = vector(((double)x / w - 0.5) * 2 * ASPECT, ((double)y / h - 0.5) * 2 * ASPECT / -ratio);

    for (i = 0; i < BALL_NUM; i++) {
        if (!isZero(balls[i].v)) {
            moving = 1;
            break;
        }
    }

    if (b == GLUT_LEFT_BUTTON && s == GLUT_DOWN/* && !moving*/)
        balls[0].v = divi(sub(point, balls[0].p), 20);
}

// マウス移動
void PassiveMotion(int x, int y) {
    struct vector point;
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);

    // ウィンドウの縦横比
    double ratio = (double)w / h;

    // マウスの座標を描画時の座標系に変換
    if (ratio > ASPECT)
        point = vector(((double)x / w - 0.5) * 2 * ratio, ((double)y / h - 0.5) * -2);
    else
        point = vector(((double)x / w - 0.5) * 2 * ASPECT, ((double)y / h - 0.5) * 2 * ASPECT / -ratio);

    cue.angle = angle(sub(point, balls[0].p)) + M_PI;
}

// 初期化
void init(void) {
    struct vector p1 = {TABLE_W / 2, 0}; // 1番玉の位置
    double r = BALL_R + 0.0001;   // ボールの間隔

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
}

// 更新
void update(void) {
    int i, j;

    // 移動
    for (i = 0; i < BALL_NUM; i++) {
        if (!balls[i].exist) continue;

        balls[i].p = add(balls[i].p, balls[i].v);
        balls[i].angle = fmod(balls[i].angle + mag(balls[i].v) / (2 * balls[i].r), 2 * M_PI);

        if (!isZero(balls[i].v))
            balls[i].dir = normal(balls[i].v);

        if (mag(balls[i].v) <= 0.001)
            balls[i].v = ZERO;
        else
            balls[i].v = mult(balls[i].v, 1 - FRICTION / mag(balls[i].v));
    }

    // ポケット判定
    pocket();

    // ボール同士の衝突
    for (i = 0; i < BALL_NUM; i++) {
        if (!balls[i].exist) continue;

        for (j = i + 1; j < BALL_NUM; j++) {
            if (!balls[j].exist) continue;

            if (dist(balls[i].p, balls[j].p) < balls[i].r + balls[j].r) {
                struct vector dir_p, dir_v, temp;
                double dist;

                // ボールの重なりを修正
                dir_p = sub(balls[i].p, balls[j].p);
                dist = mag(dir_p);
                dir_p = normal(dir_p);

                temp = mult(dir_p, (balls[i].r + balls[j].r - dist) / 2);
                balls[i].p = add(balls[i].p, temp);
                balls[j].p = sub(balls[j].p, temp);

                // 2つのボールの位置の単位ベクトル
                dir_p = normal(sub(balls[i].p, balls[j].p));

                // 2つのボールの相対速度
                dir_v = sub(balls[i].v, balls[j].v);

                // 衝突後の速度を決定
                temp = mult(dir_p, -inner(dir_p, dir_v));
                balls[i].v = mult(add(balls[i].v, temp), BALL_LOSS);
                balls[j].v = mult(sub(balls[j].v, temp), BALL_LOSS);
            }
        }
    }

    // テーブルとの衝突判定
    table.collide();
}

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

// 四角いテーブルの衝突判定
void collideSquare(void) {
    int i;

    for (i = 0; i < BALL_NUM; i++) {
        if (!balls[i].exist) continue;

        if (balls[i].p.x > TABLE_W - balls[i].r) {
            balls[i].p.x = TABLE_W - balls[i].r;
            balls[i].v.x *= -WALL_LOSS;
        }

        if (balls[i].p.x < -TABLE_W + balls[i].r) {
            balls[i].p.x = -TABLE_W + balls[i].r;
            balls[i].v.x *= -WALL_LOSS;
        }

        if (balls[i].p.y > TABLE_H - balls[i].r) {
            balls[i].p.y = TABLE_H - balls[i].r;
            balls[i].v.y *= -WALL_LOSS;
        }

        if (balls[i].p.y < -TABLE_H + balls[i].r) {
            balls[i].p.y = -TABLE_H + balls[i].r;
            balls[i].v.y *= -WALL_LOSS;
        }
    }
}

// 丸いテーブルの衝突判定
void collideCircle(void) {
    int i;

    for (i = 0; i < BALL_NUM; i++) {
        if (!balls[i].exist) continue;

        if (mag(balls[i].p) > 1 - balls[i].r) {
            balls[i].p = mult(normal(balls[i].p), 1 - balls[i].r);
            balls[i].v = mult(rotate(balls[i].v, M_PI - (angle(balls[i].v) - angle(balls[i].p)) * 2), WALL_LOSS);
        }
    }
}

// ポケットに入ったか
void pocket(void) {
    int i, j;

    for (i = 0; i < BALL_NUM; i++) {
        if (i == 0 || !balls[i].exist) continue;

        for (j = 0; j < table.pocket_num; j++) {
            if (dist(balls[i].p, table.pockets[j]) < table.pocket_r) {
                balls[i].exist = 0;
                balls[i].v = ZERO;
                break;
            }
        }
    }
}
