#include <stdio.h>
#include <math.h>

#include <time.h>
#include <sys/time.h>

#include <GL/glut.h>
#include <GL/glpng.h>

#include "billiard.h"

#define FPS    60 // フレームレート
#define ASPECT (16.0 / 9.0) // アスペクト比(幅/高さ)

int main(int argc, char *argv[]) {
    // 初期化
    glutInit(&argc, argv);
    glutInitWindowSize(540, 720);
    glutCreateWindow("Mechanical Clock");
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
    glutMainLoop();

    return 0;
}

// 画面描画
void Display(void) {
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
    struct timeval tv;

    // 次のタイマーを登録
    glutTimerFunc(1000.0 / FPS + 0.5, Timer, 0);

    // 時刻を取得
    gettimeofday(&tv, NULL);
    t = tv.tv_sec + tv.tv_usec * 1e-6;

    Display();
}

void Mouse(int b, int s, int x, int y) {
    if (b == GLUT_LEFT_BUTTON) {
        if (s == GLUT_UP) {
            printf("左ボタンアップ");
        }

        if (s == GLUT_DOWN) {
            printf("左ボタンダウン");
        }
    }

    if (b == GLUT_MIDDLE_BUTTON) {
        if (s == GLUT_UP) printf("中央ボタンアップ");
        if (s == GLUT_DOWN) printf("中央ボタンダウン");
    }

    if (b == GLUT_RIGHT_BUTTON) {
        if (s == GLUT_UP) printf("右ボタンアップ");
        if (s == GLUT_DOWN) printf("右ボタンダウン");
    }

    printf(" at (%d, %d)\n", x, y);
}

void PassiveMotion(int x, int y) {
    printf("PassiveMotion : (x, y) = (%d, %d)\n", x, y);
}

void Motion(int x, int y) {
    printf("Motion : (x, y) = (%d, %d)\n", x, y);
}
