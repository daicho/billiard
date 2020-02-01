#include <stdio.h>
#include <math.h>
#include <time.h>

#include <GL/glut.h>
#include <GL/glpng.h>

#include "billiard.h"
#include "ball.h"
#include "vector.h"
#include "shape.h"

#define FPS    60    // フレームレート
#define ASPECT 2.0   // アスペクト比 (幅/高さ)
#define BALL_R 0.04  // ボールの半径
#define CUE_W  1.536 // キューの幅
#define CUE_H  0.048 // キューの高さ

// 角度を変換
#define radian(deg) (deg * M_PI / 180.0)
#define degree(rad) (rad * 180.0 / M_PI)

// オブジェクト
struct ball prev_balls[BALL_NUM];
struct ball balls[BALL_NUM];
struct table table = {{1.75, 0.875}, 0.0896, 0.8};
struct cue cue = {1, {0, 0}, 0};
GLuint invalid_image;
GLuint highlight_image;
GLuint turn_images[2];

struct vector mouse = {0, 0};
enum status status = Stop;
int turn = 0;
int turn_left = TURN_TIME;
int break_shot = 1;
int next = 1;
int first_touch = 0;

int main(int argc, char *argv[]) {
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
    int i;
    struct vector p1 = {table.size.x / 2, 0}; // 1番玉の位置
    double r = BALL_R + 0.0001; // ボールの間隔

    // ボールを配置
    initBall(&balls[0], 0, -table.size.x / 2, 0, BALL_R);
    initBall(&balls[1], 1, p1.x, p1.y, BALL_R);
    initBall(&balls[2], 5, p1.x + r * sqrt(3), p1.y - r, BALL_R);
    initBall(&balls[3], 6, p1.x + r * sqrt(3), p1.y + r, BALL_R);
    initBall(&balls[4], 9, p1.x + r * 2 * sqrt(3), p1.y, BALL_R);
    initBall(&balls[5], 2, p1.x + r * 2 * sqrt(3), p1.y - r * 2, BALL_R);
    initBall(&balls[6], 4, p1.x + r * 2 * sqrt(3), p1.y + r * 2, BALL_R);
    initBall(&balls[7], 7, p1.x + r * 3 * sqrt(3), p1.y - r, BALL_R);
    initBall(&balls[8], 8, p1.x + r * 3 * sqrt(3), p1.y + r, BALL_R);
    initBall(&balls[9], 3, p1.x + r * 4 * sqrt(3), p1.y, BALL_R);

    // キューを配置
    cue.p = balls[0].p;

    // 画像読み込み
    table.image = pngBind("images/square.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_NEAREST, GL_NEAREST);
    cue.image = pngBind("images/cue.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_NEAREST, GL_NEAREST);
    invalid_image = pngBind("images/invalid.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_NEAREST, GL_NEAREST);
    highlight_image = pngBind("images/highlight.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_NEAREST, GL_NEAREST);
    turn_images[0] = pngBind("images/player1.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_NEAREST, GL_NEAREST);
    turn_images[1] = pngBind("images/player2.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_NEAREST, GL_NEAREST);

    for (i = 0; i < BALL_NUM; i++) {
        char fileName[32];
        sprintf(fileName, "images/%d.png", balls[i].num);
        balls[i].image = pngBind(fileName, PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_NEAREST, GL_NEAREST);
    }
}

// 更新
void update(void) {
    int i, j;

    switch (status) {
        case Stop: {
            cue.angle = angle(sub(mouse, balls[0].p));
            break;
        }

        case Move: {
            // 移動
            for (i = 0; i < BALL_NUM; i++) {
                if (balls[i].exist)
                    moveBall(&balls[i]);
            }

            // 最初に触れたボール
            if (!first_touch) {
                for (i = 1; i < BALL_NUM; i++) {
                    if (ballColliding(balls[0], balls[i])) {
                        first_touch = balls[i].num;
                        break;
                    }
                }
            }

            // ボール同士の反射
            for (i = 0; i < BALL_NUM; i++) {
                if (!balls[i].exist) continue;

                for (j = i + 1; j < BALL_NUM; j++) {
                    if (balls[j].exist)
                        reflectBall(&balls[i], &balls[j], break_shot);
                }

                // テーブルとの反射
                collideTable(table, &balls[i]);
            }

            // ポケット判定
            for (i = 0; i < BALL_NUM; i++) {
                if (balls[i].exist)
                    pocketIn(table, &balls[i]);
            }

            if (!ballMoving()) {
                // ファウルしたか
                if (balls[0].exist && first_touch == next) {
                    int pocket = 0;
                    status = Stop;

                    // ボールが落ちていたら続行
                    for (i = 0; i < BALL_NUM; i++) {
                        if (prev_balls[i].exist && !balls[i].exist) {
                            pocket = 1;
                            break;
                        }
                    }

                    if (!pocket) {
                        turn ^= 1;
                        turn_left = TURN_TIME;
                    }

                    break_shot = 0;
                    cue.p = balls[0].p;
                    cue.angle = angle(sub(mouse, balls[0].p));
                } else {
                    status = Put;
                    turn ^= 1;
                    turn_left = TURN_TIME;

                    // ボールの状態を復元
                    for (i = 0; i < BALL_NUM; i++)
                        balls[i] = prev_balls[i];

                    balls[0].p = mouse;
                    balls[0].angle = 0;
                }

                // 次に落とすべきボール
                next = 0;
                for (i = 1; i < BALL_NUM; i++) {
                    if (balls[i].exist && (balls[i].num < next || next == 0))
                        next = balls[i].num;
                }
            }

            break;
        }

        case Pull: {
            cue.power += 0.001;
            if (cue.power > 0.12)
                cue.power = 0.12;

            break;
        }

        case Put: {
            balls[0].p = mouse;
            break;
        }
    }
}

// 四角いテーブルの衝突判定
void collideTable(struct table table, struct ball *ball) {
    if (ball->p.x > table.size.x - ball->r) {
        ball->p.x = table.size.x - ball->r;
        ball->p.y = ball->p.y - tan(angle(ball->v)) * (ball->p.x - table.size.x + ball->r);
        ball->v.x *= -1;
        ball->v = mult(ball->v, table.wall_loss);
    }

    if (ball->p.x < -table.size.x + ball->r) {
        ball->p.x = -table.size.x + ball->r;
        ball->p.y = ball->p.y - tan(angle(ball->v)) * (ball->p.x + table.size.x - ball->r);
        ball->v.x *= -1;
        ball->v = mult(ball->v, table.wall_loss);
    }

    if (ball->p.y > table.size.y - balls->r) {
        ball->p.x = ball->p.x - (ball->p.y - table.size.y + ball->r) / tan(angle(ball->v));
        ball->p.y = table.size.y - balls->r;
        ball->v.y *= -1;
        ball->v = mult(ball->v, table.wall_loss);
    }

    if (ball->p.y < -table.size.y + balls->r) {
        ball->p.x = ball->p.x - (ball->p.y + table.size.y - ball->r) / tan(angle(ball->v));
        ball->p.y = -table.size.y + balls->r;
        ball->v.y *= -1;
        ball->v = mult(ball->v, table.wall_loss);
    }
}

// ポケットに入ったか
void pocketIn(struct table table, struct ball *ball) {
    int i;

    // ポケットの座標
    struct vector pockets[6] = {
        {-table.size.x, table.size.y},
        {0, table.size.y},
        {table.size.x, table.size.y},
        {-table.size.x, -table.size.y},
        {0, -table.size.y},
        {table.size.x, -table.size.y}
    };

    for (i = 0; i < 6; i++) {
        if (dist(ball->p, pockets[i]) < table.pocket_r) {
            ball->exist = 0;
            ball->v = ZERO;
            break;
        }
    }
}

// ボールが動いているか
int ballMoving(void) {
    int i;

    for (i = 0; i < BALL_NUM; i++) {
        if (!isZero(balls[i].v))
            return 1;
    }

    return 0;
}

// ボールを置けるか
int canPut(void) {
    int i;

    if (fabs(balls[0].p.x) > table.size.x - balls[0].r || fabs(balls[0].p.y) > table.size.y - balls[0].r)
        return 0;

    for (i = 1; i < BALL_NUM; i++) {
        if (ballColliding(balls[0], balls[i]))
            return 0;
    }

    return 1;
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
    double predict_min;
    struct vector predict_pos;

    GLfloat lightPos[2][4] = {
        {-1.0, 0.0, 10.0, 1.0},
        {1.0, 0.0, 10.0, 1.0}
    };

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 光源を設定
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos[0]);
    glLightfv(GL_LIGHT1, GL_POSITION, lightPos[1]);

    // テーブル
    putSprite(table.image, 0, 0, ASPECT * 2, ASPECT, 1);

    // ハイライト
    if (status != Move) {
        for (i = 1; i < BALL_NUM; i++) {
            if (balls[i].exist && next == balls[i].num) {
                putSprite(highlight_image, balls[i].p.x, balls[i].p.y, balls[i].r * 3, balls[i].r * 3, 1);
                break;
            }
        }
    }

    // 予想線
    if (status == Stop || status == Pull) {
        // テーブルとの接触
        if (sin(cue.angle) > 0)
            predict_pos = vector(balls[0].p.x - (balls[0].p.y - table.size.y + balls[0].r) / tan(cue.angle), table.size.y - balls[0].r);
        else
            predict_pos = vector(balls[0].p.x - (balls[0].p.y + table.size.y - balls[0].r) / tan(cue.angle), balls[0].r - table.size.y);

        if (fabs(predict_pos.x) < table.size.x - balls[0].r)
            predict_min = dist(balls[0].p, predict_pos);

        if (cos(cue.angle) > 0)
            predict_pos = vector(table.size.x - balls[0].r, balls[0].p.y - tan(cue.angle) * (balls[0].p.x - table.size.x + balls[0].r));
        else
            predict_pos = vector(balls[0].r - table.size.x, balls[0].p.y - tan(cue.angle) * (balls[0].p.x + table.size.x - balls[0].r));

        if (fabs(predict_pos.y) < table.size.y - balls[0].r)
            predict_min = dist(balls[0].p, predict_pos);

        // ボールとの接触
        for (i = 1; i < BALL_NUM; i++) {
            double touch;
            double predict_dist;

            if (!balls[i].exist || !cue.exist) continue;
            if (cos(angle(sub(balls[i].p, balls[0].p)) - cue.angle) < 0) continue;

            // 手玉の軌道上との距離を算出
            touch = fabs(tan(cue.angle) * (balls[i].p.x - balls[0].p.x) - balls[i].p.y + balls[0].p.y) / mag(vector(tan(cue.angle), 1));

            if (touch < balls[0].r + balls[i].r) {
                // 衝突予想地点を算出
                predict_dist = dist(balls[0].p, balls[i].p) * cos(angle(sub(balls[i].p, balls[0].p)) - cue.angle) - sqrt(pow(balls[0].r + balls[i].r, 2) - pow(touch, 2));

                if (predict_dist < predict_min) {
                    target = i;
                    predict_min = predict_dist;
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

        drawCircle(predict_min, 0, balls[0].r, 32);
        glPopMatrix();
        glEnable(GL_LIGHTING);
    }

    // ボール
    for (i = BALL_NUM - 1; i >= 0; i--) {
        if (balls[i].exist)
            drawBall(balls[i]);
    }

    // 無効マーク
    if (status == Put && !canPut())
        putSprite(invalid_image, balls[0].p.x, balls[0].p.y, balls[0].r * 2, balls[0].r * 2, 1);

    if ((status == Stop || status == Pull) && target && balls[target].num != next)
        putSprite(invalid_image, balls[target].p.x, balls[target].p.y, balls[target].r * 2, balls[target].r * 2, 1);

    // キュー
    if (cue.exist) {
        glPushMatrix();
        glTranslated(cue.p.x, cue.p.y, 0);
        glRotated(degree(cue.angle) + 180, 0, 0, 1);
        putSprite(cue.image, balls[0].r + CUE_W / 2 + cue.power * 2, 0, CUE_W, CUE_H, 1);
        glPopMatrix();
    }

    // ターン表示
    if (turn_left > 0) {
        double step = 1 - 2.0 * turn_left / TURN_TIME;
        turn_left--;
        putSprite(turn_images[turn], 0, 0, ASPECT * 2, ASPECT, 1 - pow(step, 4));
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

    mouse = convertPoint(x, y);

    if (b == GLUT_LEFT_BUTTON) {
        switch (status) {
            case Stop: {
                if (s == GLUT_DOWN) {
                    status = Pull;
                    cue.angle = angle(sub(mouse, balls[0].p));
                }

                break;
            }

            case Pull: {
                if (s == GLUT_UP) {
                    status = Move;

                    // 状態を保持
                    for (i = 0; i < BALL_NUM; i++)
                        prev_balls[i] = balls[i];

                    balls[0].v = mult(vector(cos(cue.angle), sin(cue.angle)), cue.power);
                    cue.power = 0;
                    first_touch = 0;
                }

                break;
            }

            case Put: {
                if (s == GLUT_DOWN && canPut()) {
                    status = Stop;
                    cue.p = balls[0].p = mouse;
                }

                break;
            }

            default:
                break;
        }
    }
}

// マウス移動
void PassiveMotion(int x, int y) {
    mouse = convertPoint(x, y);
}
