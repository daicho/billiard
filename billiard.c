#include <stdio.h>
#include <math.h>
#include <time.h>

#include <GL/glut.h>
#include <GL/glpng.h>

#include "billiard.h"
#include "ball.h"
#include "vector.h"
#include "shape.h"

#define FPS       60     // フレームレート
#define ASPECT    2      // アスペクト比 (幅/高さ)
#define BALL_R    0.0393 // ボールの半径
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
struct cue cue = {1, {0, 0}, 0};
int pulling = 0;

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
}

// 更新
void update(void) {
    int i, j;

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
            ball->exist = 0;
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

    // ボール
    for (i = 0; i < BALL_NUM; i++) {
        if (balls[i].exist)
            drawBall(balls[i]);
    }

    // キュー
    if (cue.exist) {
        glPushMatrix();
        glTranslated(balls[0].p.x, balls[0].p.y, 0);
        glRotated(degree(cue.angle), 0, 0, 1);
        putSprite(cue.image, balls[0].r + CUE_W / 2, 0, balls[0].r * 2, CUE_W, CUE_H);
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
    int i;
    struct vector point = convertPoint(x, y);

    if (b == GLUT_LEFT_BUTTON) {
        // 動いているボールがあるか
        int moving = 0;

        for (i = 0; i < BALL_NUM; i++) {
            if (!isZero(balls[i].v)) {
                moving = 1;
                break;
            }
        }

        if (s == GLUT_DOWN && !moving) {
            pulling = 1;
        }

        if (s == GLUT_UP) {
            if (pulling) {
                balls[0].v = divi(sub(point, balls[0].p), 20);
            }
        }
    }
}

// マウス移動
void PassiveMotion(int x, int y) {
    struct vector point = convertPoint(x, y); 
    cue.angle = angle(sub(point, balls[0].p)) + M_PI;
}
