#include <stdio.h>
#include <math.h>
#include <float.h>

#include "vector.h"

// ゼロベクトル
const struct vector ZERO = {0, 0};

// ベクトルを生成
struct vector vector(double x, double y) {
    struct vector a = {x, y};
    return a;
}

// 加算
struct vector add(struct vector a, const struct vector b) {
    return vector(a.x + b.x, a.y + b.y);
}

// 減算
struct vector sub(struct vector a, const struct vector b) {
    return vector(a.x - b.x, a.y - b.y);
}

// 乗算
struct vector mult(struct vector a, double r) {
    return vector(a.x * r, a.y * r);
}

// 除算
struct vector divi(struct vector a, double r) {
    return vector(a.x / r, a.y / r);
}

// 回転
struct vector rotate(struct vector a, double r) {
    return vector(a.x * cos(r) - a.y * sin(r), a.x * sin(r) + a.y * cos(r));
}

// 正規化
struct vector normal(struct vector a) {
    return divi(a, mag(a));
}

// 大きさ
double mag(struct vector a) {
    return sqrt(pow(a.x, 2) + pow(a.y, 2));
}

// ベクトルの角度
double angle(struct vector a) {
    return atan2(a.y, a.x);
}

// 2つのベクトルのなす角度
double angle2(struct vector a, struct vector b) {
    return acos(inner(a, b) / (mag(a) * mag(b)));
}

// 距離
double dist(struct vector a, struct vector b) {
    return mag(sub(a, b));
}

// 内積
double inner(struct vector a, struct vector b) {
    return a.x * b.x + a.y * b.y;
}

// ゼロベクトルか
int isZero(struct vector a) {
    return mag(a) <= DBL_EPSILON;
}
