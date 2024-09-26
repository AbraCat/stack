#ifndef STACK_H
#define STACK_H

#define STACK_NDEBUG

#define stDump(file, st) stDumpFn(file, st, __FILE__, __LINE__, __FUNCTION__)
#define handleError(error) handleErrorFn(error, __FILE__, __LINE__, __FUNCTION__)

#ifdef STACK_NDEBUG

#define ST_ON_DEBUG(expr) ;
#define stAssert(expr) ;
#define stCtor(st, capacity) stCtorNDebug(st, capacity)

#else

#define ST_ON_DEBUG(expr) expr
#define stAssert(expr) stAssertFn(expr, #expr, __FILE__, __LINE__, __FUNCTION__)
#define stCtor(st, capacity) stCtorDebug(st, capacity, __FILE__, __LINE__, __FUNCTION__)

#endif //STACK_NDEBUG

typedef int StackElem;

struct Stack
{
    ST_ON_DEBUG
    (
        const char* file_name;
        int line_born;
        const char* func_born;
    )

    volatile int size;
    int capacity;
    StackElem* data;
};

enum StError
{
    ERR_STACK_UNDERFLOW = 1,
    ERR_NULL_STACK,
    ERR_BAD_SIZE,
    ERR_NOMEM
};

void handleErrorFn(int error, const char* file, int line, const char* func);
void stAssertFn(int expr, const char* str_expr, const char* file, int line, const char* func);

int stCtorNDebug( Stack* st, int capacity);
int stCtorDebug(Stack* st, int capacity, const char* file_name, int line_born, const char* func_born);

void stDtor(Stack* st);

int stPush(Stack* st, StackElem elem);
int stPop(Stack* st, StackElem* elem);

const char* stStrError(int error);
int stError(Stack* st);
void stDumpFn(FILE* file, Stack* st, const char* file_name, int line, const char* func_name);

#endif //STACK_H