#include <gtest/gtest.h>

extern "C" {
#include <vl/vl_linked_list.h>

    int list_growth_test() {
        vl_linked_list* list = vlListNew(sizeof(int));

        for(int i = 0; i < 1024; i++){
            vlListPushBack(list, &i);
        }

        const int result = list->length == 1024 && (list->tail != list->head);

        vlListDelete(list);

        return result;
    }

    int list_sort_test(){
        const int numElements = 10;
        const int elementsSorted[] = {3, 4, 14, 43, 54, 62, 63, 81, 86, 91};
        const int elements[] = {62, 14, 43, 91, 54, 4, 3, 63, 86, 81};
        int result = 1;
        int i = 0;

        vl_linked_list* list = vlListNew(sizeof(int));

        for(; i < numElements; i++)
            vlListPushBack(list, elements + i);

        vlListSort(list, vlCompareInt);

        i = 0;
        VL_LIST_FOREACH(list, curIter)
            result = result && (elementsSorted[i++] == *((const int*)(vlListSample(list, curIter))));

        vlListDelete(list);

        return result;
    }

    int list_iterate_test(int reverse){
        vl_linked_list* list = vlListNew(sizeof(int));

        int expect = 0;
        for(int i = 0; i < 1024; i++){
            vlListPushBack(list, &i);
            expect += i;
        }

        int sum = 0;

        if(reverse){
            VL_LIST_FOREACH_REVERSE(list, curIter){
                sum += *((int*) vlListSample(list, curIter));
            }
        }else{
            VL_LIST_FOREACH(list, curIter){
                sum += *((int*) vlListSample(list, curIter));
            }
        }

        vlListDelete(list);

        return sum == expect;
    }

    int list_inline_insert(){
        vl_linked_list* list = vlListNew(sizeof(int));

        int set_size = 3;
        int set[] = {1,3,5};
        int val_set[] = {2,4};

        int expect[] = {1,2,3,4,5};

        for(int i = 0 ; i < set_size; i++){
            vlListPushBack(list, &(set[i]));
        }

        vl_list_iter centerIter = vlListNext(list, list->head);
        vlListInsertBefore(list, centerIter, &val_set[0]);
        vlListInsertAfter(list, centerIter, &val_set[1]);

        int result = 1;
        int i = 0;
        VL_LIST_FOREACH(list, curIter){
            const int value = *((int*) vlListSample(list, curIter));

            result = result && (value == expect[i]);
            i++;
        }

        vlListDelete(list);

        return result;
    }
}

TEST(linked_list, growth){
    ASSERT_EQ(list_growth_test(), 1);
}

TEST(linked_list, iterate_forward){
    ASSERT_EQ(list_iterate_test(false), 1);
}

TEST(linked_list, sort){
    ASSERT_EQ(list_sort_test(), 1);
}

TEST(linked_list, iterate_backward){
    ASSERT_EQ(list_iterate_test(true), 1);
}

TEST(linked_list, inline_insert){
    ASSERT_EQ(list_inline_insert(), 1);
}