#include "set.h"
#include <vl/vl_set.h>

vl_bool_t vlTestSetGrowth() {
    const int set_size = 1024;

    vl_set *set = vlSetNew(sizeof(int), vlCompareInt);

    for (int i = 0; i < set_size; i++) {
        vlSetInsert(set, &i);
    }

    const vl_bool_t result = set->totalElements == set_size;

    vlSetDelete(set);

    return result;
}

vl_bool_t vlTestSetOrder() {
    const int set_size = 10;
    const int data[] = {6, 2, 9, 1, 3, 0, 4, 7, 5, 8};
    const int data_sorted[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    vl_set *set = vlSetNew(sizeof(int), vlCompareInt);

    for (int i = 0; i < set_size; i++) {
        vlSetInsert(set, (void *) &(data[i]));
    }

    int i = 0;
    vl_bool_t result = VL_TRUE;
    VL_SET_FOREACH(set, curIter) {
        const int value = *((int *) vlSetSample(set, curIter));
        result = result && (data_sorted[i] == value);
        i++;
    }

    vlSetDelete(set);

    return result;
}

vl_bool_t vlTestSetIterate(vl_bool_t reverse) {
    const int set_size = 1000;

    vl_set *set = vlSetNew(sizeof(int), vlCompareInt);

    int expect = 0;

    for (int i = 0; i < set_size; i++) {
        vlSetInsert(set, &i);
        expect += i;
    }

    int sum = 0;
    vl_uint32_t total = 0;

    if (reverse) {
        VL_SET_FOREACH_REVERSE(set, curIter) {
            sum += *((int *) vlSetSample(set, curIter));
            total++;
        }
    } else {
        VL_SET_FOREACH(set, curIter) {
            sum += *((int *) vlSetSample(set, curIter));
            total++;
        }
    }

    const vl_bool_t result = (total == set_size) && (total == set->totalElements) && (sum == expect);

    vlSetDelete(set);

    return result;
}
