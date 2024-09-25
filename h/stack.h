#ifndef STACK_H
#define STACK_H

#define ST_ASSERT_CHECK(error) stAssertFn(error, __FILE__, __LINE__, __FUNCTION__)

#define stDump(file, st) stDumpFn(file, st, __FILE__, __LINE__, __FUNCTION__)

#ifdef STACK_NDEBUG

#define ST_ON_DEBUG (expr) ;

#define stCtor(st, capacity) stCtorNDebug(st, capacity)

#define ST_ASSERT_RUN(error) (error)
#define ST_ASSERT_NRUN(error) ;

#else

#define ST_ON_DEBUG(expr) expr

#define stCtor(st, capacity) stCtorDebug(st, capacity, __FILE__, __LINE__, __FUNCTION__)

#define ST_ASSERT_RUN(error) stAssertFn(error, __FILE__, __LINE__, __FUNCTION__)
#define ST_ASSERT_NRUN(error) stAssertFn(error, __FILE__, __LINE__, __FUNCTION__)

#endif //STACK_NDEBUG

typedef int StackElem;

struct Stack
{
    ST_ON_DEBUG(const char* file_name;)
    ST_ON_DEBUG(int line_born;)
    ST_ON_DEBUG(const char* func_born;)

    int size, capacity;
    StackElem* data;
};

enum StError
{
    ERR_STACK_UNDERFLOW,
    ERR_NULL_STACK,
    ERR_BAD_SIZE
};

void stAssertFn(int error, const char* file, int line, const char* func);

int stCtorNDebug(Stack* st, int capacity);
int stCtorDebug(Stack* st, int capacity, const char* file_name, int line_born, const char* func_born);

void stDtor(Stack* st);

int stPush(Stack* st, StackElem elem);
int stPop(Stack* st, StackElem* elem);

const char* stStrError(int error);
int stError(Stack* st);
void stDumpFn(FILE* file, Stack* st, const char* file_name, int line, const char* func_name);

#endif //STACK_H