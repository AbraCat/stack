#include <stdio.h>
#include <stdlib.h>

#include <stack.h>

int stMin(int a, int b)
{
    return a < b ? a : b;
}
int stMax(int a, int b)
{
    return a > b ? a : b;
}

#define DESCR_(err_code) case ERR_ ## err_code:\
    return "ERR_" #err_code;

const char* stStrError(int error)
{
    switch (error)
    {
        DESCR_(ASSERT)
        DESCR_(STACK_UNDERFLOW)
        DESCR_(NULL_STACK)
        DESCR_(BAD_SIZE)
        DESCR_(NOMEM)
        DESCR_(POISON);
        DESCR_(BAD_CANARY)
        DESCR_(BAD_HASH)

        default:
            return "Unknown error";
    }
}

#undef DESCR_

void handleErrorFn(int error, const char* file, int line, const char* func)
{
    if (!error)
        return;

    printf("Error: %s\nat %s:%d function: %s\n", stStrError(error), file, line, func);
    
    exit(error);
}

void stAssertFn(int expr, const char* str_expr, const char* file, int line, const char* func)
{
    if (expr)
        return;

    printf("Assrtion failed: %s\nat %s:%d function: %s\n", str_expr, file, line, func);

    exit(ERR_ASSERT);
}

int stCtorNDebug(Stack* st, int capacity)
{
    if (st == NULL)
        return ERR_NULL_STACK;

    st->size = 0;
    st->capacity = capacity;
    st->data = NULL;

    return resize(st, capacity);
}

int stCtorDebug(Stack* st, int capacity, const char* file_name, int line_born, const char* func_born)
{
    if (st == NULL)
        return ERR_NULL_STACK;

    ST_ON_DEBUG
    (
        st->left_st_can = st->right_st_can = can_val;

        st->file_name = file_name;
        st->line_born = line_born;
        st->func_born = func_born;
    )

    st->size = 0;
    st->capacity = capacity;
    st->data = NULL;

    int error = resize(st, capacity);

    if (error)
        return error;

    stUpdateHash(st);

    stAssert(stError(st) == 0);

    return 0;
}

void stDtor(Stack* st)
{
    free(st->data);
}

int hashFn(char* arr, int size)
{

    int hash = 5381;

    if (arr == NULL)
        return hash;

    for (int i = 0; i < size; ++i)
    {
        hash = (hash << 5) + hash + arr[i];
    }

    return hash;
}

void stUpdateHash(Stack* st)
{
    ST_ON_DEBUG
    (
    stAssert(st != NULL);

    st->st_hash = st->data_hash = 0;

    st->st_hash = hashFn((char*)st, sizeof(Stack));


    if (st->data == NULL)
    {
        st->data_hash = hashFn((char*)st->data, st->capacity);

        return;
    }

    st->data_hash = hashFn((char*)(st->data - 1), (st->capacity + 2) * sizeof(StackElem));
    )
}

int resize(Stack* st, int new_capacity)
{
    stAssert(st != NULL);

    st->capacity = new_capacity;

    if (new_capacity == 0)
    {
        st->data = NULL;
        return 0;
    }

    ST_ON_RELEASE
    (
        st->data = (StackElem*)realloc(st->data, new_capacity * sizeof(StackElem));
    )

    ST_ON_DEBUG
    (
        st->data = (StackElem*)realloc(st->data == NULL ? NULL : st->data - 1, (new_capacity + 2) * sizeof(StackElem)) + 1;
    )

    if (st->data == NULL)
    {
        return ERR_NOMEM;
    }

    ST_ON_DEBUG
    (
        st->data[-1] = st->data[new_capacity] = can_val;

        for (int i = st->size; i < new_capacity; ++i)
        {
            st->data[i] = poison_val;
        }
    )

    return 0;
}

int stPush(Stack* st, StackElem elem)
{
    stAssert(stError(st) == 0);

    if (st->size == st->capacity)
    {
        resize(st, st->capacity == 0 ? 1 : st->capacity * 2);
    }

    st->data[st->size++] = elem;

    stUpdateHash(st);

    stAssert(stError(st) == 0);

    return 0;
}

int stPop(Stack* st, StackElem* elem)
{
    stAssert(stError(st) == 0);

    if (st->size == 0)
        return ERR_STACK_UNDERFLOW;

    *elem = st->data[--(st->size)];
    
    ST_ON_DEBUG
    (
        st->data[st->size] = poison_val;
    )

    if (st->size <= st->capacity / 4)
    {
        resize(st, st->capacity / 2);
    }

    stUpdateHash(st);

    stAssert(stError(st) == 0);

    return 0;
}

int stError(Stack* st)
{
    if (st == NULL)
    {
        return ERR_NULL_STACK;
    }

    if (st->data == NULL && st->capacity != 0)
    {
        return ERR_NULL_STACK;
    }

    ST_ON_DEBUG
    (
        if (st->file_name == NULL || st->func_born == NULL)
        {
            return ERR_NULL_STACK;
        }
    )

    if (0 > st->size || st->size > st->capacity)
        return ERR_BAD_SIZE;

    ST_ON_DEBUG
    (
        for (int i = st->size; i < st->capacity; ++i)
        {
            if (st->data[i] != poison_val)
            {
                return ERR_POISON;
            }
        }

        if (st->left_st_can != can_val || st->right_st_can != can_val)
        {
            return ERR_BAD_CANARY;
        }

        if (st->data != NULL && (st->data[-1] != can_val || st->data[st->capacity] != can_val))
        {
            return ERR_BAD_CANARY;
        }



        const int st_hash_saved = st->st_hash, data_hash_saved = st->data_hash;

        stUpdateHash(st);

        int good_hash = (st->st_hash == st_hash_saved && st->data_hash == data_hash_saved);

        st->st_hash = st_hash_saved;
        st->data_hash = data_hash_saved;

        if (!good_hash)
            return ERR_BAD_HASH;
    )

    return 0;
}

void stDumpFn(FILE* file, Stack* st, const char* file_name, int line, const char* func_name)
{
    static const int max_n_elem = 20;

    if (file == NULL || file_name == NULL || func_name == NULL)
        return;

    ST_ON_RELEASE
    (
        fprintf(file, "Stack [0x%p] at %s:%d (function %s)\n\n", 
        st, file_name, line, func_name);
    )

    if (st == NULL)
        return;

    ST_ON_DEBUG
    (
        fprintf(file, "Stack [0x%p] at %s:%d (function %s) born at %s:%d (function %s)\n\n", 
        st, file_name, line, func_name, 
        st->file_name == NULL ? "NULL" : st->file_name, st->line_born, st->func_born == NULL ? "NULL" : st->func_born);
    )

    fprintf(file, "size = %d\ncapacity = %d\n\n", st->size, st->capacity);

    ST_ON_DEBUG
    (
        fprintf(file, "left_st_can = 0x%X\nright_st_can = 0x%X\nleft_data_can = 0x%X\nright_data_can = 0x%X\n\n", 
        st->left_st_can, st->right_st_can, st->data[-1], st->data[st->capacity]);

        fprintf(file, "st_hash = %d\ndata_hash = %d\n\n", st->st_hash, st->data_hash);
    )



    fprintf(file, "data [0x%p]:\n\n", st->data);

    int too_big = 0;

    if (0 > st->size || st->size > st->capacity)
    {
        fprintf(file, "%s", "Warning: size and/or capacity are incorrect\n\n");
    }
    else if (st->capacity > max_n_elem)
    {
        too_big = 1;
        if (st->size > max_n_elem)
        {
            fprintf(file, "%s", "Warning: size is too big\n\n");
        }
        else
        {
            fprintf(file, "%s", "Warning: capacity is too big\n\n");
        }
    }

    if (st->data == NULL)
        return;

    int rgt1 = stMin(max_n_elem, stMin(st->size, st->capacity));
    int rgt2 = stMin(max_n_elem, st->capacity);

    for (int i = 0; i < rgt1; ++i)
    {
        fprintf(file, "* [%d]: %d\n", i, st->data[i]);
    }
    for (int i = stMax(st->size, 0); i < rgt2; ++i)
    {
        fprintf(file, "  [%d]: %d\n", i, st->data[i]);
    }
    if (too_big)
    {
        fprintf(file, "...\n");
    }

    putc('\n', file);
}