#include <stdio.h>
#include <stdlib.h>

#include <stack.h>
#include <utils.h>
#include <colors.h>



StackElem poison_val = 3452663;
StackElem canary_val = 0xB3A61C;



#define DESCR_(err_code) case ERR_ ## err_code: return "ERR_" #err_code;

const char* stStrError(stErrCode error)
{
    switch (error)
    {
        DESCR_(OK);
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

void handleErrFn(stErrCode error, const char* file, int line, const char* func)
{
    if (!error)
        return;

    printf("%sError: %s%s\nat %s:%d function: %s\n", RED, stStrError(error), DEFAULT, file, line, func);
    
    exit(error);
}

void stAssertFn(int expr, const char* str_expr, const char* file, int line, const char* func)
{
    if (expr)
        return;

    printf("%sAssrtion failed: %s%s\nat %s:%d function: %s\n", RED, str_expr, DEFAULT, file, line, func);

    exit(ERR_ASSERT);
}

stErrCode stCtorNDebug(Stack* st, int capacity)
{
    if (st == NULL)
        return ERR_NULL_STACK;

    st->size = 0;
    st->capacity = capacity;
    st->data = NULL;

    ST_ON_CANARY
    (
        st->left_st_canary = st->right_st_canary = canary_val;
    )

    returnErr(resize(st, capacity));

    stUpdateHash(st);

    returnErr(stErr(st));
    return ERR_OK;
}

stErrCode stCtorDebug(Stack* st, int capacity, const char* file_name, int line_born, const char* func_born)
{
    if (st == NULL)
        return ERR_NULL_STACK;

    ST_ON_DEBUG
    (
        st->file_name = file_name;
        st->line_born = line_born;
        st->func_born = func_born;
    )

    ST_ON_CANARY
    (
        st->left_st_canary = st->right_st_canary = canary_val;
    )

    st->size = 0;
    st->capacity = capacity;
    st->data = NULL;

    returnErr(resize(st, capacity));

    stUpdateHash(st);

    returnErr(stErr(st));
    return ERR_OK;
}

void stDtor(Stack* st)
{
    if (st == NULL)
        return;
        
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

void stUpdateHashFn(Stack* st)
{
    ST_ON_HASH
    (
    stAssert(st != NULL);

    st->st_hash = st->data_hash = 0;

    ST_ON_NO_CANARY
    (
    st->st_hash = hashFn((char*)st, sizeof(Stack));
    )

    ST_ON_CANARY
    (
    st->st_hash = hashFn((char*)st + sizeof(StackElem), sizeof(Stack) - 2 * sizeof(StackElem));
    )

    st->data_hash = hashFn((char*)(st->data), st->capacity * sizeof(StackElem));
    )
}

stErrCode resize(Stack* st, int new_capacity)
{
    stAssert(st != NULL);

    st->capacity = new_capacity;

    if (new_capacity == 0)
    {
        st->data = NULL;
        return ERR_OK;
    }

    ST_ON_NO_CANARY
    (
        st->data = (StackElem*)realloc(st->data, new_capacity * sizeof(StackElem));
    )

    ST_ON_CANARY
    (
        st->data = (StackElem*)realloc(st->data == NULL ? NULL : st->data - 1, (new_capacity + 2) * sizeof(StackElem)) + 1;
    )

    if (st->data == NULL)
    {
        return ERR_NOMEM;
    }

    ST_ON_CANARY
    (
        st->data[-1] = st->data[new_capacity] = canary_val;
    )

    ST_ON_DEBUG
    (
        for (int i = st->size; i < new_capacity; ++i)
        {
            st->data[i] = poison_val;
        }
    )

    return ERR_OK;
}

stErrCode stPush(Stack* st, StackElem elem)
{
    returnErr(stErr(st));

    if (st->size == st->capacity)
    {
        resize(st, st->capacity == 0 ? 1 : st->capacity * 2);
    }

    st->data[st->size++] = elem;

    stUpdateHash(st);

    returnErr(stErr(st));

    return ERR_OK;
}

stErrCode stPop(Stack* st, StackElem* elem)
{
    returnErr(stErr(st));

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

    returnErr(stErr(st));

    return ERR_OK;
}

stErrCode stErr(Stack* st)
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
    )

    ST_ON_CANARY
    (
        if (st->left_st_canary != canary_val || st->right_st_canary != canary_val)
        {
            return ERR_BAD_CANARY;
        }

        if (st->data != NULL && (st->data[-1] != canary_val || st->data[st->capacity] != canary_val))
        {
            return ERR_BAD_CANARY;
        }
    )



    ST_ON_HASH
    (
        const int st_hash_saved = st->st_hash, data_hash_saved = st->data_hash;

        stUpdateHash(st);

        int good_hash = (st->st_hash == st_hash_saved && st->data_hash == data_hash_saved);

        st->st_hash = st_hash_saved;
        st->data_hash = data_hash_saved;

        if (!good_hash)
            return ERR_BAD_HASH;
    )

    return ERR_OK;
}

void stDumpFn(FILE* file, Stack* st, const char* file_name, int line, const char* func_name) //colors
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

    ST_ON_CANARY
    (
        fprintf(file, "left_st_canary = 0x%X\nright_st_canary = 0x%X\nleft_data_canary = 0x%X\nright_data_canary = 0x%X\n\n", 
        st->left_st_canary, st->right_st_canary, st->data[-1], st->data[st->capacity]);
    )

    ST_ON_HASH
    (
        fprintf(file, "st_hash = %d\ndata_hash = %d\n\n", st->st_hash, st->data_hash);
    )



    fprintf(file, "data [0x%p]:\n\n", st->data);

    int too_big = 0;

    if (0 > st->size || st->size > st->capacity)
    {
        fprintf(file, "%s%s%s", RED, "Warning: size and/or capacity are incorrect\n\n", DEFAULT);
    }
    else if (st->capacity > max_n_elem)
    {
        too_big = 1;
        if (st->size > max_n_elem)
        {
            fprintf(file, "%s%s%s", RED, "Warning: size is too big\n\n", DEFAULT);
        }
        else
        {
            fprintf(file, "%s%s%s", RED, "Warning: capacity is too big\n\n", DEFAULT);
        }
    }

    if (st->data == NULL)
        return;

    int rgt1 = myMin(max_n_elem, myMin(st->size, st->capacity));
    int rgt2 = myMin(max_n_elem, st->capacity);

    for (int i = 0; i < rgt1; ++i)
    {
        fprintf(file, "* [%d]: %d\n", i, st->data[i]);
    }
    for (int i = myMax(st->size, 0); i < rgt2; ++i)
    {
        fprintf(file, "  [%d]: %d\n", i, st->data[i]);
    }
    if (too_big)
    {
        fprintf(file, "...\n");
    }

    putc('\n', file);
}