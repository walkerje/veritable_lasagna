#include <gtest/gtest.h>

extern "C" {
#include <vl/vl_set.h>

int set_growth_test() {
    const int set_size = 1024;

    vl_set *set = vlSetNew(sizeof(int), vlCompareInt);

    for (int i = 0; i < set_size; i++) {
        vlSetInsert(set, &i);
    }

    const int result = set->totalElements == set_size;

    vlSetDelete(set);

    return result;
}

int set_order_test() {
    const int set_size = 10;
    const int data[] = {6, 2, 9, 1, 3, 0, 4, 7, 5, 8};
    const int data_sorted[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    vl_set *set = vlSetNew(sizeof(int), vlCompareInt);

    for (int i = 0; i < set_size; i++) {
        vlSetInsert(set, (void *) &(data[i]));
    }

    int i = 0;
    int result = 1;
    VL_SET_FOREACH(set, curIter) {
        const int value = *((int *) vlSetSample(set, curIter));
        result = result && (data_sorted[i] == value);
        i++;
    }

    vlSetDelete(set);

    return result;
}

int set_iterate_test(int forward) {
    const int set_size = 1000;

    vl_set *set = vlSetNew(sizeof(int), vlCompareInt);

    int expect = 0;

    for (int i = 0; i < set_size; i++) {
        vlSetInsert(set, &i);
        expect += i;
    }

    int sum = 0;
    uint32_t total = 0;

    if (forward) {
        VL_SET_FOREACH(set, curIter) {
            sum += *((int *) vlSetSample(set, curIter));
            total++;
        }
    } else {
        VL_SET_FOREACH_REVERSE(set, curIter) {
            sum += *((int *) vlSetSample(set, curIter));
            total++;
        }
    }

    const int result = (total == set_size) && (total == set->totalElements) && (sum == expect);

    vlSetDelete(set);

    return result;
}
}

TEST(set, growth) {
    ASSERT_EQ(set_growth_test(), 1);
}

TEST(set, order) {
    ASSERT_EQ(set_order_test(), 1);
}

TEST(set, iterate_forward) {
    ASSERT_EQ(set_iterate_test(true), 1);
}

TEST(set, iterate_reverse) {
    ASSERT_EQ(set_iterate_test(false), 1);
}