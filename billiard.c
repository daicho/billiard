#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <windows.h>

#include <GL/glut.h>
#include <GL/glpng.h>

#include "billiard.h"
#include "table.h"
#include "ball.h"
#include "vector.h"
#include "shape.h"

#define FPS         60    // フレームレート
#define ASPECT      2.0   // アスペクト比 (幅/高さ)
#define BALL_R      0.04  // 球の半径
#define CUE_W       1.536 // キューの幅
#define CUE_H       0.048 // キューの高さ
#define TURN_TIME   120   // ターン表示の時間
#define RESULT_TIME 240   // リザルト表示の時間

// 角度を変換
#define radian(deg) (rad * M_PI / 180.0)
#define degree(rad) (rad * 180.0 / M_PI)

// オブジェクト
struct ball prev_balls[BALL_NUM];                  // 前回の球
struct ball balls[BALL_NUM];                       // 球
struct table table = {{1.75, 0.875}, 0.0896, 0.8}; // 台
struct cue cue;                                    // キュー

// 画像
GLuint title_image;       // タイトル
GLuint vshuman_images[2]; // 人vs人のボタン
GLuint vscpu_images[2];   // 人vsCPUのボタン
GLuint invalid_image;     // 無効マーク
GLuint highlight_image;   // ハイライト
GLuint turn_images[4];    // ターン表示
GLuint win_images[4];     // 勝者表示

struct vector mouse = {ASPECT, 1}; // マウス座標
enum scene scene = Title;          // シーン
enum status status;                // 状態
enum turn turn;                    // ターン
int turn_left;                     // ターン表示の残り時間
int win_left;                      // 勝者表示の残り時間
int break_shot;                    // ブレイクショットか
int next;                          // 次に狙う球
int first_touch;                   // 最初に当たった球
double cpu_power;                  // CPUが球を弾く強さ

int main(int argc, char *argv[]) {
    int i;
    char fileName[FILENAME_MAX];

    srand(time(NULL));

    // 初期化
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA);

#ifdef FULL_SCREEN
    glutEnterGameMode();
#else
    glutInitWindowSize(960, 480);
    glutCreateWindow("REAL BILLIARD");
#endif

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
    glutKeyboardFunc(Keyboard);

    // 画像読み込み
    title_image = pngBind("images/title.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_NEAREST, GL_NEAREST);

    for (i = 0; i < 2; i++) {
        sprintf(fileName, "images/vshuman_%d.png", i);
        vshuman_images[i] = pngBind(fileName, PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_NEAREST, GL_NEAREST);

        sprintf(fileName, "images/vscpu_%d.png", i);
        vscpu_images[i] = pngBind(fileName, PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_NEAREST, GL_NEAREST);
    }

    for (i = 0; i < BALL_NUM; i++) {
        sprintf(fileName, "images/ball_%d.png", i);
        balls[i].image = pngBind(fileName, PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_NEAREST, GL_NEAREST);
    }

    table.image = pngBind("images/table.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_NEAREST, GL_NEAREST);
    cue.image = pngBind("images/cue.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_NEAREST, GL_NEAREST);
    invalid_image = pngBind("images/invalid.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_NEAREST, GL_NEAREST);
    highlight_image = pngBind("images/highlight.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_NEAREST, GL_NEAREST);

    for (i = 0; i < 4; i++) {
        sprintf(fileName, "images/turn_%d.png", i);
        turn_images[i] = pngBind(fileName, PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_NEAREST, GL_NEAREST);

        sprintf(fileName, "images/win_%d.png", i);
        win_images[i] = pngBind(fileName, PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_NEAREST, GL_NEAREST);
    }

    // 音声ファイル読み込み
    mciSendString("open shot.mp3 alias shot", NULL, 0, NULL);
    mciSendString("open collide.mp3 alias collide", NULL, 0, NULL);
    mciSendString("open pocket.mp3 alias pocket", NULL, 0, NULL);

    // メインループ開始
    glutMainLoop();

    return 0;
}

// 初期化
void init(void) {
    struct vector p1 = {table.size.x / 2, 0}; // 1番玉の位置
    double r = BALL_R + 0.0001;               // 球の間隔

    // 球を配置
    initBall(&balls[0], 0, vector(-table.size.x / 2, 0), BALL_R);
    initBall(&balls[1], 1, vector(p1.x, p1.y), BALL_R);
    initBall(&balls[2], 2, vector(p1.x + r * 2 * sqrt(3), p1.y - r * 2), BALL_R);
    initBall(&balls[3], 3, vector(p1.x + r * 4 * sqrt(3), p1.y), BALL_R);
    initBall(&balls[4], 4, vector(p1.x + r * 2 * sqrt(3), p1.y + r * 2), BALL_R);
    initBall(&balls[5], 5, vector(p1.x + r * sqrt(3), p1.y - r), BALL_R);
    initBall(&balls[6], 6, vector(p1.x + r * sqrt(3), p1.y + r), BALL_R);
    initBall(&balls[7], 7, vector(p1.x + r * 3 * sqrt(3), p1.y - r), BALL_R);
    initBall(&balls[8], 8, vector(p1.x + r * 3 * sqrt(3), p1.y + r), BALL_R);
    initBall(&balls[9], 9, vector(p1.x + r * 2 * sqrt(3), p1.y), BALL_R);

    // キューを配置
    cue.p = balls[0].p;
    cue.angle = angle(sub(mouse, balls[0].p));
    cue.power = 0;
}

// 更新
void update(void) {
    int i, j;

    // 表示タイマーを更新
    if (status == Result) {
        if (win_left > 0)
            win_left--;
        else
            scene = Title;
    } else {
        if (turn_left > 0)
            turn_left--;
    }

    switch (status) {
        // 静止中
        case Stop: {
            if ((turn == CPU || 1)) {
                int target = 0;
                double angle_min = 1;
                struct vector temp;

                // ポケットの座標
                struct vector pockets[6] = {
                    {-table.size.x, table.size.y},
                    {0, table.size.y},
                    {table.size.x, table.size.y},
                    {-table.size.x, -table.size.y},
                    {0, -table.size.y},
                    {table.size.x, -table.size.y}
                };

                if (break_shot) {
                    cue.angle = angle(sub(balls[next].p, balls[0].p)) + ((double)rand() / RAND_MAX - 0.5) * 0.01;
                    cpu_power = 0.12;
                } else {
                    // 一番入れやすいポケットを狙う
                    for (i = 0; i < 6; i++) {
                        double angle = cos(angle2(sub(pockets[i], balls[next].p), sub(balls[0].p, balls[next].p)));

                        if (angle < angle_min) {
                            angle_min = angle;
                            target = i;
                        }
                    }

                    temp = sub(add(balls[next].p, mult(normal(sub(balls[next].p, pockets[target])), balls[0].r + balls[next].r)), balls[0].p);
                    cue.angle = angle(temp) + ((double)rand() / RAND_MAX - 0.5) * 0.005 / mag(temp);

                    // 球を弾く強さを決定
                    cpu_power = pow(dist(balls[0].p, balls[next].p) + dist(balls[next].p, pockets[target]), 0.5) * 0.04;
                    if (cpu_power > 0.12)
                        cpu_power = 0.12;
                }

                status = Pull;
            } else {
                // キューをマウスの方向に向ける
                cue.angle = angle(sub(mouse, balls[0].p));
            }

            break;
        }

        // 手球を配置中
        case Put: {
            if ((turn == CPU || 1)) {
                // 置ける場所にランダムに配置
                do {
                    double angle = 2 * M_PI * rand() / RAND_MAX;
                    balls[0].p = add(balls[next].p, mult(vector(cos(angle), sin(angle)), 0.3 * rand() / RAND_MAX + 0.2));
                } while (!canPut());

                cue.p = balls[0].p;
                status = Stop;
            } else {
                balls[0].p = mouse;
            }

            break;
        }

        // キューを引いている
        case Pull: {
            // 力を溜める
            cue.power += 0.001;
            if (cue.power > 0.12)
                cue.power = 0.12;

            if ((turn == CPU || 1) && cue.power >= cpu_power) {
                // 音声再生
                mciSendString("play shot from 0", NULL, 0, NULL);

                // 状態を保持
                for (i = 0; i < BALL_NUM; i++)
                    prev_balls[i] = balls[i];

                // 手球に初速度を与える
                balls[0].v = mult(vector(cos(cue.angle), sin(cue.angle)), cue.power);
                cue.power = 0;
                first_touch = 0;
                status = Move;
            }

            break;
        }

        case Move: {
            // 移動
            for (i = 0; i < BALL_NUM; i++) {
                if (balls[i].exist)
                    moveBall(&balls[i]);
            }

            // 最初に触れた球
            if (!first_touch) {
                for (i = 1; i < BALL_NUM; i++) {
                    if (balls[i].exist && ballColliding(balls[0], balls[i])) {
                        first_touch = balls[i].num;
                        break;
                    }
                }
            }

            // 球の反射
            for (i = 0; i < BALL_NUM; i++) {
                if (!balls[i].exist) continue;

                // 球同士の反射
                for (j = i + 1; j < BALL_NUM; j++) {
                    if (balls[j].exist)
                        reflectBall(&balls[i], &balls[j], break_shot);
                }

                // 台との反射
                reflectTable(table, &balls[i]);
            }

            // ポケットイン
            for (i = 0; i < BALL_NUM; i++) {
                if (balls[i].exist)
                    pocketIn(table, &balls[i]);
            }

            // 球が止まったら
            if (!ballMoving()) {
                // ファウルしていなかったら
                if (balls[0].exist && first_touch == next) {
                    // 9番球が入っていなかったら
                    if (balls[9].exist) {
                        int pocket = 0;
                        status = Stop;

                        // いずれかの球が落ちていたら続行
                        for (i = 0; i < BALL_NUM; i++) {
                            if (prev_balls[i].exist && !balls[i].exist) {
                                pocket = 1;
                                break;
                            }
                        }

                        // 球が落ちていなかったらターンチェンジ
                        if (!pocket) {
                            turn ^= 1;
                            turn_left = TURN_TIME;
                        }

                        break_shot = 0;
                        cue.p = balls[0].p;
                        cue.angle = angle(sub(mouse, balls[0].p));
                    } else {
                        // 9番球が入ったら勝者表示
                        status = Result;
                        win_left = RESULT_TIME;
                    }
                } else {
                    // ファウルだったらターンチェンジして手球の自由配置
                    status = Put;
                    turn ^= 1;
                    turn_left = TURN_TIME;

                    // 球の状態を復元
                    for (i = 0; i < BALL_NUM; i++)
                        balls[i] = prev_balls[i];

                    balls[0].p = mouse;
                    balls[0].angle = 0;
                }

                // 次に落とすべき球
                next = 0;
                for (i = 1; i < BALL_NUM; i++) {
                    if (balls[i].exist && (balls[i].num < next || next == 0))
                        next = balls[i].num;
                }
            }

            break;
        }

        default:
            break;
    }
}

// 球が動いているか
int ballMoving(void) {
    int i;

    for (i = 0; i < BALL_NUM; i++) {
        if (!isZero(balls[i].v))
            return 1;
    }

    return 0;
}

// 球を置けるか
int canPut(void) {
    int i;

    if (fabs(balls[0].p.x) > table.size.x - balls[0].r || fabs(balls[0].p.y) > table.size.y - balls[0].r)
        return 0;

    if (pocketTouching(table, balls[0]))
        return 0;

    for (i = 1; i < BALL_NUM; i++) {
        if (balls[i].exist && ballColliding(balls[0], balls[i]))
            return 0;
    }

    return 1;
}

// ピクセル座標を描画時の座標に変換
struct vector convertPoint(int x, int y) {
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);

    // ウィンドウの縦横比
    double ratio = (double)w / h;

    // 座標を変換
    if (ratio > ASPECT)
        return vector(((double)x / w - 0.5) * 2 * ratio, ((double)y / h - 0.5) * -2);
    else
        return vector(((double)x / w - 0.5) * 2 * ASPECT, ((double)y / h - 0.5) * 2 * ASPECT / -ratio);
}

// 画面描画
void Display(void) {
    int i;
    int target = 0;
    double predict_min;
    struct vector predict_pos;

    // 光源の座標
    GLfloat lightPos[2][4] = {
        {-1.0, 0.0, 10.0, 1.0},
        {1.0, 0.0, 10.0, 1.0}
    };

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (scene == Title) {
        // タイトル画面
        putSprite(title_image, 0, 0, ASPECT * 2, 2, 1);

        if (fabs(mouse.x) <= ASPECT / 2 && -0.25 <= mouse.y && mouse.y <= 0.25)
            putSprite(vshuman_images[1], 0, 0, ASPECT, 0.5, 1);
        else
            putSprite(vshuman_images[0], 0, 0, ASPECT, 0.5, 1);

        if (fabs(mouse.x) <= ASPECT / 2 && -0.875 <= mouse.y && mouse.y <= -0.375)
            putSprite(vscpu_images[1], 0, -0.625, ASPECT, 0.5, 1);
        else
            putSprite(vscpu_images[0], 0, -0.625, ASPECT, 0.5, 1);
    } else {
        // 光源を設定
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos[0]);
        glLightfv(GL_LIGHT1, GL_POSITION, lightPos[1]);

        // 台
        putSprite(table.image, 0, 0, ASPECT * 2, 2, 1);

        // ハイライト
        if (status == Stop || status == Pull || status == Put) {
            for (i = 1; i < BALL_NUM; i++) {
                if (balls[i].exist && next == balls[i].num) {
                    putSprite(highlight_image, balls[i].p.x, balls[i].p.y, balls[i].r * 3, balls[i].r * 3, 1);
                    break;
                }
            }
        }

        // 予測線
        if (status == Stop || status == Pull) {
            // 台との接触
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

            // 球との接触
            for (i = 1; i < BALL_NUM; i++) {
                double touch;
                double predict_dist;

                if (!balls[i].exist || cos(angle(sub(balls[i].p, balls[0].p)) - cue.angle) < 0) continue;

                // 手球の軌道上との距離を算出
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

        // 球
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
        glPushMatrix();
        glTranslated(cue.p.x, cue.p.y, 0);
        glRotated(degree(cue.angle) + 180, 0, 0, 1);
        putSprite(cue.image, balls[0].r + CUE_W / 2 + cue.power * 2, 0, CUE_W, CUE_H, 1);
        glPopMatrix();

        if (status == Result) {
            // 結果表示
            if (win_left > 0) {
                double step = 1 - 2.0 * win_left / RESULT_TIME;
                putSprite(win_images[turn], 0, 0, ASPECT, 0.5, 1 - pow(step, 4));
            }
        } else {
            // ターン表示
            if (turn_left > 0) {
                double step = 1 - 2.0 * turn_left / TURN_TIME;
                putSprite(turn_images[turn], 0, 0, ASPECT, 0.5, 1 - pow(step, 4));
            }
        }
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

    if (scene == Game)
        update();

    glutPostRedisplay();
}

// マウスクリック
void Mouse(int button, int stat, int x, int y) {
    int i;

    mouse = convertPoint(x, y);

    if (button == GLUT_LEFT_BUTTON) {
        if (scene == Title) {
            if (stat == GLUT_UP) {
                // 人vs人ボタンクリック
                if (fabs(mouse.x) <= ASPECT / 2 && -0.25 <= mouse.y && mouse.y <= 0.25) {
                    scene = Game;
                    turn = Player1;
                }

                // 人vsCPUボタンクリック
                if (fabs(mouse.x) <= ASPECT / 2 && -0.875 <= mouse.y && mouse.y <= -0.375) {
                    scene = Game;
                    turn = Player;
                }

                // 初期化
                if (scene == Game) {
                    status = Stop;
                    break_shot = 1;
                    next = 1;
                    turn_left = TURN_TIME;
                    init();
                }
            }
        } else {
            // CPU操作時はクリック無効
            if ((turn == CPU || 1))
                return;

            switch (status) {
                // 静止中
                case Stop: {
                    if (stat == GLUT_DOWN) {
                        status = Pull;
                        cue.angle = angle(sub(mouse, balls[0].p));
                    }

                    break;
                }

                // キューを引いている
                case Pull: {
                    if (stat == GLUT_UP) {
                        // 音声再生
                        mciSendString("play shot from 0", NULL, 0, NULL);

                        // 状態を保持
                        for (i = 0; i < BALL_NUM; i++)
                            prev_balls[i] = balls[i];

                        // 手球に初速度を与える
                        balls[0].v = mult(vector(cos(cue.angle), sin(cue.angle)), cue.power);
                        cue.power = 0;
                        first_touch = 0;
                        status = Move;
                    }

                    break;
                }

                // 手球配置中
                case Put: {
                    if (stat == GLUT_DOWN && canPut()) {
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
}

// マウス移動
void PassiveMotion(int x, int y) {
    mouse = convertPoint(x, y);
}

// キーボード押下
void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'q': {
            exit(0);
            break;
        }

        case 'r': {
            scene = Title;
            break;
        }
    }
}
