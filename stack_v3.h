typedef double stack_elem_t;
#define STACK_ELEM   "%lg"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>   // size_t

#define STACK_POISON     (NAN)
#define CANARY_VALUE     (1e308)

// ============================================================
// Режим отладки
// ============================================================
#ifdef ONDEBUG
    #define ON_DBG(...) __VA_ARGS__
#else
    #define ON_DBG(...)
#endif


enum StackError
{
    STACK_OK            = 0,
    STACK_NULL_PTR      = 1 << 0,
    STACK_MEMORY_ERROR  = 1 << 1,
    STACK_UNDERFLOW     = 1 << 2,
    STACK_OVERFLOW      = 1 << 3,
    STACK_BAD_CAPACITY  = 1 << 4,
    STACK_LEFT_CANARY   = 1 << 5,
    STACK_RIGHT_CANARY  = 1 << 6,
    STACK_DATA_NULL     = 1 << 8,
};

// ============================================================
// Структура стека
// ============================================================
struct stack_t
{
#ifdef ONDEBUG
    const char* name;   //  имя стека
    const char* file;
    int         line;
    const char* func;   //  функция создания
#endif

    stack_elem_t* data;        // сырой буфер: [канарейка][данные][канарейка]
    stack_elem_t* info_data;   // данные (указатель в середину data)
    size_t        size;
    size_t        capacity;
    stack_elem_t* pointer;
};

// ============================================================
// Прототипчики
// ============================================================
int          StackVerify    (struct stack_t* stk);

void         StackDump       (struct stack_t* stk, int err ON_DBG(, const char* file, int line, const char* func));

void         StackInit       (struct stack_t* stk, size_t capacity, int* err, ...);

void         StackResize     (struct stack_t* stk, size_t new_capacity, int* err);
void         StackPush       (struct stack_t* stk, stack_elem_t value, int* err);
stack_elem_t StackPop        (struct stack_t* stk, int* err);

void         StackDestroy    (struct stack_t* stk);

void         PrintfErr       (int err, struct stack_t* stk);

void         PrintfStackElem ( struct stack_t* stk);

void         PrintfCanary    (struct stack_t* stk);

// ============================================================
// обёрточки
// ============================================================
#ifdef ONDEBUG

    #define STACK_INIT(stk, capacity, err)                          \
        StackInit((stk), (capacity), (err),                         \
                      #stk, __FILE__, __LINE__, __func__)

    #define STACK_DUMP(stk, err)                                    \
        StackDump((stk), (err), __FILE__, __LINE__, __func__)

#else
    #define STACK_INIT(stk, capacity, err)                          \
        StackInit((stk), (capacity), (err))
    #define STACK_DUMP(stk, err)                                    \
        StackDump((stk), (err))
#endif