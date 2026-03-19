#include "hashtable.h"
#include <vl/vl_hashtable.h>
#include <string.h>
#include <stdio.h>

vl_bool_t vlTestHashTableCollision() {
    vl_hashtable *table = vlHashTableNew(vlHashString);

    //some mock keys, which have known collisions with fnv1a64 hash algo
    const char *str1 = "gMPflVXtwGDXbIhP73TX";
    const char *str2 = "LtHf1prlU1bCeYZEdqWf";
    const int len1 = (int)strlen(str1);
    const int len2 = (int)strlen(str2);

    //claim spots for our items with colliding keys
    vlHashTableInsert(table, str1, len1, sizeof(int));
    vlHashTableInsert(table, str2, len2, sizeof(int));

    //now find them with the same colliding keys, and assign them values.
    const vl_hash_iter iter1 = vlHashTableFind(table, str1, len1);
    const vl_hash_iter iter2 = vlHashTableFind(table, str2, len2);

    (*(int *) vlHashTableSampleValue(table, iter1, NULL)) = 1;
    (*(int *) vlHashTableSampleValue(table, iter2, NULL)) = 2;

    int sum = 0;
    //sum our values
    VL_HASHTABLE_FOREACH(table, curIter) {
        sum += *((int *) vlHashTableSampleValue(table, curIter, NULL));
    }

    //ensures unique locations and retained values upon collision...
    const vl_bool_t result = (sum == 3) && (iter1 != iter2);

    vlHashTableDelete(table);

    return result;
}

vl_bool_t vlTestHashTableRemove() {
    vl_hashtable *table = vlHashTableNew(vlHashInt);

    const int expect = 0;
    int sum = 0;

    for (int i = 1; i <= 10; i++) {
        *((int *) vlHashTableSampleValue(table, vlHashTableInsert(table, &i, sizeof(int), sizeof(int)), NULL)) = i;
    }

    while (table->totalElements > 0) {
        vlHashTableRemoveIter(table, vlHashTableFront(table));
    }

    VL_HASHTABLE_FOREACH(table, curIter) {
        sum += *((int *) vlHashTableSampleValue(table, curIter, NULL));
    }

    vlHashTableDelete(table);

    return sum == expect;
}

vl_bool_t vlTestHashTableInsert(vl_uint32_t set_size, vl_bool_t reserved) {
    vl_hashtable *table = vlHashTableNew(vlHashInt);

    if (reserved)
        vlHashTableReserve(table, 1000, VL_MB(4));

    for (vl_uint32_t i = 1; i <= set_size; i++) {
        *((int *) vlHashTableSampleValue(table, vlHashTableInsert(table, &i, sizeof(int), sizeof(int)), NULL)) =
                (int) set_size - i;
    }

    const vl_bool_t result = table->totalElements == set_size;
    vlHashTableDelete(table);

    return result;
}

vl_bool_t vlTestHashTableIterate(vl_int_t set_size, vl_int_t rounds, vl_bool_t reserved) {
    int expect = 0;
    int sum = 0;

    vl_hashtable *table = vlHashTableNew(vlHashInt);

    if (reserved)
        vlHashTableReserve(table, 1000, (vl_memsize_t)sizeof(int) * set_size);

    for (int r = 0; r < rounds; r++) {
        for (int i = 0; i < set_size; i++) {
            *((int *) vlHashTableSampleValue(table, vlHashTableInsert(table, &i, sizeof(int), sizeof(int)), NULL)) = i;
            expect += i;
        }

        VL_HASHTABLE_FOREACH(table, curIter) {
            int val = *((int *) vlHashTableSampleValue(table, curIter, NULL));
            sum += val;
        }

        if (sum == expect) {
            expect = 0;
            sum = 0;
            vlHashTableClear(table);
        } else break;
    }

    vlHashTableDelete(table);

    return sum == 0;
}

vl_bool_t vlTestHashTableRealWorld() {
    //String keys, float values.
    vl_hashtable *wealthTable = vlHashTableNew(vlHashString);

    const int numEntries = 5;
    const char *keys[] = {
            "McLovin",
            "Supercop",
            "Napoleon",
            "Terminator",
            "Loch Ness Monster"
    };
    const float values[] = {12.05f, 5.84f, 910.63f, 711.42f, 3.50f};

    for (int i = 0; i < numEntries; i++) {
        const char *key = keys[i];
        const float value = values[i];
        const int keyLen = (int)strlen(key) + 1; //+1 to preserve null terminator

        //Insert the key and claim the memory for the value.
        const vl_hash_iter iter = vlHashTableInsert(wealthTable, key, keyLen, sizeof(float));
        //Then assign the value to the table's memory.
        *((float *) vlHashTableSampleValue(wealthTable, iter, NULL)) = value;
    }

    VL_HASHTABLE_FOREACH(wealthTable, curIter) {
        //size of key and value, in bytes
        vl_memsize_t keyLen, valLen;
        const char *key = (const char *) vlHashTableSampleKey(wealthTable, curIter, &keyLen);
        const float val = *((float *) vlHashTableSampleValue(wealthTable, curIter, &valLen));

        (void)key; (void)val;
        // printf("%s has %.2f$ in the bank!\n", key, val);
    }

    vlHashTableDelete(wealthTable);
    return VL_TRUE;
}
