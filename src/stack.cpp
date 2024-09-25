#include <stdio.h>
#include <stdlib.h>

#include <stack.h>

#define DESCR_(err_code) case ERR_ ## err_code:\
    return "ERR_" #err_code "\n";

const char* stStrError(int error)
{
    switch (error)
    {
        DESCR_(STACK_UNDERFLOW)
        DESCR_(NULL_STACK)
        DESCR_(BAD_SIZE)

        default:
            return "Unknown error";
    }
}

#undef DESCR_

void stAssertFn(int error, const char* file, int line, const char* func)
{
    if (!error)
        return;

    printf("Error: %s\n", stStrError(error));
    printf("file: %s line: %d function: %s\n", file, line, func);
    
    exit(error);
}

int stCtorNDebug(Stack* st, int capacity)
{
    st->size = 0;
    st->capacity = capacity;
    st->data = (StackElem*)calloc(capacity, sizeof(StackElem));

    return 0;
}

int stCtorDebug(Stack* st, int capacity, const char* file_name, int line_born, const char* func_born)
{
    st->file_name = file_name;
    st->line_born = line_born;
    st->func_born = func_born;

    st->size = 0;
    st->capacity = capacity;
    st->data = (StackElem*)calloc(capacity, sizeof(StackElem));

    return 0;
}

void stDtor(Stack* st)
{
    free(st->data);
}

int stPush(Stack* st, StackElem elem)
{
    ST_ASSERT_NRUN(stError(st));

    if (st->capacity == 0)
    {
        st->data = (StackElem*)realloc(st->data, sizeof(StackElem));
        
        st->capacity = 1;
    }

    else if (st->size == st->capacity)
    {
        st->data = (StackElem*)realloc(st->data, st->capacity * 2 * sizeof(StackElem));
        st->capacity *= 2;
    }

    st->data[st->size++] = elem;

    printf("push(): size = %d cap = %d\n", st->size, st->capacity);

    ST_ASSERT_NRUN(stError(st));

    return 0;
}

int stPop(Stack* st, StackElem* elem)
{
    ST_ASSERT_NRUN(stError(st));

    if (st->size == 0)
        return ERR_STACK_UNDERFLOW;

    *elem = st->data[--(st->size)];

    if (st->size <= st->capacity / 4)
    {
        st->data = (StackElem*)realloc(st->data, (st->capacity / 2) * sizeof(StackElem));
        st->capacity /= 2;
    }
    printf("pop(): size = %d cap = %d\n", st->size, st->capacity);

    ST_ASSERT_NRUN(stError(st));

    return 0;
}

int stError(Stack* st)
{
    if (st->capacity == 0)
    {
        return 0;
    }

    if (st == NULL)
    {
        printf("aaa\n");
        return ERR_NULL_STACK;
    }

    if (st->data == NULL ST_ON_DEBUG(|| st->file_name == NULL || st->func_born == NULL))
    {
        printf("bbb\n");
        return ERR_NULL_STACK;
    }

    if (0 > st->size || st->size > st->capacity)
        return ERR_BAD_SIZE;

    return 0;
}

void stDumpFn(FILE* file, Stack* st, const char* file_name, int line, const char* func_name)
{
    if (file == NULL || file_name == NULL || func_name == NULL)
        return;

    if (st == NULL)
        fprintf(file, "Stack [NULL]\n\n");

    #ifdef STACK_NDEBUG

    fprintf(file, "Stack %p at %s:%d (function %s)\n\n", 
    st, file_name, line, func_name);

    #else

    fprintf(file, "Stack %p at %s:%d (function %s) born at %s:%d (function %s)\n\n", 
    st, file_name, line, func_name, st->file_name, st->line_born, st->func_born);

    #endif //STACK_NDEBUG

    fprintf(file, "size = %d capacity = %d\n\n", st->size, st->capacity);

    for (int i = 0; i < (st->size <= st->capacity ? st->size : st->capacity); ++i)
    {
        fprintf(file, "* [%d]: %d\n", i, st->data[i]);
    }

    for (int i = st->size; i < st->capacity; ++i)
    {
        fprintf(file, "[%d]: %d\n", i, st->data[i]);
    }

    putc('\n', file);
}