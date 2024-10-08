#include <stdio.h>
#include <string.h> 

#include <stack.h>
#include <utils.h>
#include <colors.h>

int main(int argc, const char* argv[])
{
    Stack st = {};
    handleErr(stCtor(&st, 3));
    
    int n_cycles = 1, max_size = 5;

    for (int j = 0; j < n_cycles; ++j)
    {
        for (int i = 0; i < max_size; ++i)
        {
            handleErr(stPush(&st, i));
        }

        stDump(stdout, &st);

        int x = 0;
        for (int i = 0; i < max_size; ++i)
        {
            handleErr(stPop(&st, &x));
        }
    }

    stDtor(&st);

    // stDump(stdout, &st);

    return 0;
}