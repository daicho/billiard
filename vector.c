#include <stdio.h>
#include <math.h>
#include <float.h>

#include "vector.h"

const struct vector ZERO = {0, 0};

// 値をセット
void set(struct vector *a, double x, double y) {
    a->x = x;
    a->y = y;
}

// ベクトルを生成
struct vector vector(double x, double y) {
    struct vector a = {x, y};
    return a;
}

// 加算 (再代入)
void add(struct vector *a, const struct vector b) {
    a->x += b.x;
    a->y += b.y;
}

// 減算 (再代入)
void sub(struct vector *a, const struct vector b) {
    a->x -= b.x;
    a->y -= b.y;
}

// 乗算 (再代入)
void mult(struct vector *a, double r) {
    a->x *= r;
    a->y *= r;
}

// 除算 (再代入)
void divi(struct vector *a, double r) {
    a->x /= r;
    a->y /= r;
}

// 加算
struct vector plus(struct vector a, const struct vector b) {
    struct vector c = a;
    add(&c, b);
    return c;
}

// 減算
struct vector minus(struct vector a, const struct vector b) {
    struct vector c = a;
    sub(&c, b);
    return c;
}

// 乗算
struct vector times(struct vector a, double r) {
    struct vector c = a;
    mult(&c, r);
    return c;
}

// 除算
struct vector split(struct vector a, double r) {
    struct vector c = a;
    divi(&c, r);
    return c;
}

// 回転
void rotate(struct vector *a, double r) {
    set(a, a->x * cos(r) - a->y * sin(r), a->x * sin(r) + a->y * cos(r));
}

// 正規化
void normal(struct vector *a) {
    divi(a, mag(*a));
}

// 大きさ
double mag(struct vector a) {
    return sqrt(pow(a.x, 2) + pow(a.y, 2));
}

// ベクトルの角度
double angle(struct vector a) {
    return atan2(a.y, a.x);
}

// 距離
double dist(struct vector a, struct vector b) {
    return mag(minus(a, b));
}

// 内積
double inner(struct vector a, struct vector b) {
    return a.x * b.x + a.y * b.y;
}

// ゼロベクトルか
int isZero(struct vector a) {
    return mag(a) <= DBL_EPSILON;
}

// ベクトルの内容を表示
void printVector(struct vector a) {
    printf("(%f, %f)\n", a.x, a.y);
}
