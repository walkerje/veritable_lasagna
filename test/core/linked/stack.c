#include "stack.h"
#include <vl/vl_stack.h>
#include <stdio.h>
#include <stdint.h>

static void stack_hanoi_move(vl_stack *rods[3], int a, int b) {
    const vl_bool_t aEmpty = vlStackEmpty(rods[a]);
    const vl_bool_t bEmpty = vlStackEmpty(rods[b]);

    const int32_t aTop = !aEmpty ? *(int32_t *) (vlStackPeek(rods[a])) : INT32_MIN;
    const int32_t bTop = !bEmpty ? *(int32_t *) (vlStackPeek(rods[b])) : INT32_MIN;

    if (!aEmpty)
        vlStackPop(rods[a]);
    if (!bEmpty)
        vlStackPop(rods[b]);

    if (aTop == INT32_MIN) {
        vlStackPushValue(rods[a], &bTop, sizeof(int32_t));
    } else if (bTop == INT32_MIN) {
        vlStackPushValue(rods[b], &aTop, sizeof(int32_t));
    } else if (aTop > bTop) {
        vlStackPushValue(rods[a], &aTop, sizeof(int32_t));
        vlStackPushValue(rods[a], &bTop, sizeof(int32_t));
    } else {
        vlStackPushValue(rods[b], &bTop, sizeof(int32_t));
        vlStackPushValue(rods[b], &aTop, sizeof(int32_t));
    }
}

vl_bool_t vlTestStackHanoi() {
    const int disks = 5;
    vl_stack *rods[3];
    rods[0] = vlStackNew();
    rods[1] = vlStackNew();
    rods[2] = vlStackNew();

    int src = 0, aux = 1, dest = 2;
    for (int i = disks; i >= 1; i--) {
        int val = i;
        vlStackPushValue(rods[src], &val, sizeof(int));
    }

    const int totalMoves = (1 << disks) - 1;

    if (disks % 2 == 0) {
        aux = 2;
        dest = 1;
    }

    for (int i = 1; i <= totalMoves; i++) {
        switch (i % 3) {
            case 0:
                stack_hanoi_move(rods, aux, dest);
                break;
            case 1:
                stack_hanoi_move(rods, src, dest);
                break;
            default:
                stack_hanoi_move(rods, src, aux);
                break;
        }
    }

    const vl_memsize_t destSize = rods[dest]->depth;

    vlStackDelete(rods[0]);
    vlStackDelete(rods[1]);
    vlStackDelete(rods[2]);

    return (vl_bool_t)(destSize == (vl_memsize_t)disks);
}
