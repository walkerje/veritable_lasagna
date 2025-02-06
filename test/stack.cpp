#include <gtest/gtest.h>

extern "C" {
#include <vl/vl_stack.h>

    void stack_hanoi_move(vl_stack** stacks, int a, int b){
        const int aEmpty = vlStackEmpty(stacks[a]);
        const int bEmpty = vlStackEmpty(stacks[b]);

        const int aTop = !aEmpty ? *(int*)(vlStackPeek(stacks[a])) : INT32_MIN;
        const int bTop = !bEmpty ? *(int*)(vlStackPeek(stacks[b])) : INT32_MIN;

        if(!aEmpty)
            vlStackPop(stacks[a]);
        if(!bEmpty)
            vlStackPop(stacks[b]);

        if(aTop == INT32_MIN){
            vlStackPushValue(stacks[a], &bTop, sizeof(int));
            printf("Moved disk #%d from %c to %c\n", bTop, (char)(b+'A'), (char)(a+'A'));
        }else if(bTop == INT32_MIN){
            vlStackPushValue(stacks[b], &aTop, sizeof(int));
            printf("Moved disk #%d from %c to %c\n", aTop, (char)(a+'A'), (char)(b+'A'));
        }else if(aTop > bTop){
            vlStackPushValue(stacks[a], &aTop, sizeof(int));
            vlStackPushValue(stacks[a], &bTop, sizeof(int));
            printf("Moved disk #%d from %c to %c\n", bTop, (char)(b+'A'), (char)(a+'A'));
        }else{
            vlStackPushValue(stacks[b], &bTop, sizeof(int));
            vlStackPushValue(stacks[b], &aTop, sizeof(int));
            printf("Moved disk #%d from %c to %c\n", aTop, (char)(a+'A'), (char)(b+'A'));
        }
    }

    int stack_hanoi(int disks){
        vl_stack* rods[3];
        rods[0] = vlStackNew();
        rods[1] = vlStackNew();
        rods[2] = vlStackNew();

        int src = 0, aux = 1, dest = 2;
        for(int i = disks; i >= 1; i--)
            vlStackPushValue(rods[src], &i, sizeof(int));

        const int totalMoves = (1 << disks) - 1;

        if(disks % 2 == 0){
            aux = 2;
            dest = 1;
        }

        for(int i = 1; i <= totalMoves; i++){
            switch(i % 3){
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

        const int destSize = rods[dest]->depth;

        vlStackDelete(rods[0]);
        vlStackDelete(rods[1]);
        vlStackDelete(rods[2]);

        return destSize == disks;
    }
}

//the only test specified for stacks.
//solve the tower of hanoi puzzle.
TEST(stack, hanoi){
    EXPECT_TRUE(stack_hanoi(5));
}