//#define STACK_NDEBUG

#include <stdio.h>

#include <stack.h>

int main(int argc, const char* argv[])
{
    Stack st;
    ST_ASSERT_RUN(stCtor(&st, 0));

    int n_cycles = 1, max_size = 10;

    for (int j = 0; j < n_cycles; ++j)
    {
        for (int i = 0; i < max_size; ++i)
        {
            ST_ASSERT_RUN(stPush(&st, i));
        }

        stDump(stdout, &st);

        int x = 0;
        for (int i = 0; i < max_size; ++i)
        {
            ST_ASSERT_RUN(stPop(&st, &x));
            printf("%d ", x);
        }

        putchar('\n');
    }

    printf("size = %d cap = %d\n", st.size, st.capacity);

    stDtor(&st);
    return 0;
}