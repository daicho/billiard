#ifndef VECTOR_H
#define VECTOR_H

// 二次元ベクトル
struct vector {
    double x;
    double y;
};

void set(struct vector *, double, double);
void add(struct vector *, const struct vector *);
void sub(struct vector *, const struct vector *);
void mult(struct vector *, double);
void divi(struct vector *, double);
double mag(struct vector);
double dist(struct vector, struct vector);
double inner(struct vector, struct vector);
void show(struct vector);

#endif
