#ifndef VECTOR_H
#define VECTOR_H

// 二次元ベクトル
struct vector {
    double x;
    double y;
};

struct vector vector(double, double);

struct vector add(struct vector, struct vector);
struct vector sub(struct vector, struct vector);
struct vector mult(struct vector, double);
struct vector divi(struct vector, double);

struct vector rotate(struct vector, double);
struct vector normal(struct vector);

double mag(struct vector);
double angle(struct vector);
double dist(struct vector, struct vector);
double inner(struct vector, struct vector);
int isZero(struct vector);

void printVector(struct vector);

extern const struct vector ZERO;

#endif
