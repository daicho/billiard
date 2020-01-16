#include <stdio.h>
#include <math.h>

#include "vector.h"

// 値をセット
void set(struct vector *a, double x, double y) {
    a->x = x;
    a->y = y;
}

// 加算
void add(struct vector *a, const struct vector *b) {
    a->x += b->x;
    a->y += b->y;
}

// 減算
void sub(struct vector *a, const struct vector *b) {
    a->x -= b->x;
    a->y -= b->y;
}

// 乗算
void mult(struct vector *a, double r) {
    a->x *= r;
    a->y *= r;
}

// 除算
void divi(struct vector *a, double r) {
    a->x /= r;
    a->y /= r;
}

// 大きさ
double mag(struct vector a) {
    return sqrt(pow(a.x, 2) + pow(a.y, 2));
}

// 距離
double dist(struct vector a, struct vector b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

// 内積
double inner(struct vector a, struct vector b) {
    return a.x * b.x + a.y * b.y;
}

// ベクトルの内容を表示
void show(struct vector a) {
    printf("(%f, %f)\n", a.x, a.y);
}
