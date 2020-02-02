// 16426 福澤大地

#include <math.h>

#include "table.h"
#include "ball.h"
#include "vector.h"

// 台との反射
void reflectTable(struct table table, struct ball *ball) {
    // 右側
    if (ball->p.x > table.size.x - ball->r) {
        ball->p.x = table.size.x - ball->r;
        ball->p.y = ball->p.y - tan(angle(ball->v)) * (ball->p.x - table.size.x + ball->r);
        ball->v.x *= -1;
        ball->v = mult(ball->v, table.wall_loss);
    }

    // 左側
    if (ball->p.x < -table.size.x + ball->r) {
        ball->p.x = -table.size.x + ball->r;
        ball->p.y = ball->p.y - tan(angle(ball->v)) * (ball->p.x + table.size.x - ball->r);
        ball->v.x *= -1;
        ball->v = mult(ball->v, table.wall_loss);
    }

    // 上側
    if (ball->p.y > table.size.y - ball->r) {
        ball->p.x = ball->p.x - (ball->p.y - table.size.y + ball->r) / tan(angle(ball->v));
        ball->p.y = table.size.y - ball->r;
        ball->v.y *= -1;
        ball->v = mult(ball->v, table.wall_loss);
    }

    // 下側
    if (ball->p.y < -table.size.y + ball->r) {
        ball->p.x = ball->p.x - (ball->p.y + table.size.y - ball->r) / tan(angle(ball->v));
        ball->p.y = -table.size.y + ball->r;
        ball->v.y *= -1;
        ball->v = mult(ball->v, table.wall_loss);
    }
}

// ポケットに入るか
int pocketTouching(struct table table, struct ball ball) {
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
        if (dist(ball.p, pockets[i]) < table.pocket_r)
            return 1;
    }

    return 0;
}

// ポケットに入れる
void pocketIn(struct table table, struct ball *ball) {
    if (pocketTouching(table, *ball)) {
        ball->exist = 0;
        ball->v = ZERO;
    }
}
