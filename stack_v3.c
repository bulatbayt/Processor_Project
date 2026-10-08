#include "stack_v3.h"



// ---------- Init ----------
void StackInit(struct stack_t* stk, size_t capacity, int* err, ...)
{
    assert(stk != NULL);
    assert(err != NULL);

    va_list args;
    va_start(args, err);

#ifdef ONDEBUG
    stk->name = va_arg(args, const char*);
    stk->file = va_arg(args, const char*);
    stk->line = va_arg(args, int);
    stk->func = va_arg(args, const char*);
#endif

    va_end(args);

    if (capacity == 0)
    {
        *err = STACK_BAD_CAPACITY;
        return;
    }

    stk->data = calloc((capacity + 2), sizeof(stack_elem_t)); //

    if (stk->data == NULL)
    {
        stk->capacity = 0;
        *err = STACK_MEMORY_ERROR;
        return;
    }

    // Левая канарейка
    stk->data[0] = CANARY_VALUE;

    // Данные — сразу после канарейки
    stk->info_data = stk->data + 1;
    stk->pointer = stk->info_data;

    // Правая канарейка
    stk->data[capacity + 1] = CANARY_VALUE;

    // Заливаем всё poison-ом
    for (size_t i = 0; i < capacity; ++i)
        stk->info_data[i] = STACK_POISON;

    stk->size     = 0;
    stk->capacity = capacity;
    *err = STACK_OK;
}

// ---------- Verify ----------
int StackVerify(struct stack_t* stk)
{
    if (stk == NULL)
        return STACK_NULL_PTR;

    int err = STACK_OK;

    if (stk->capacity == 0) 
        err |= STACK_BAD_CAPACITY;

    if (stk->size > stk->capacity)
        err |= STACK_OVERFLOW;

    if (stk->capacity > 0 && stk->data == NULL)
        err |= STACK_DATA_NULL;

    if (stk->data != NULL)
    {
        stack_elem_t left  = stk->data[0];
        stack_elem_t right = stk->data[stk->capacity + 1];

        if (left  != CANARY_VALUE) err |= STACK_LEFT_CANARY;
        if (right != CANARY_VALUE) err |= STACK_RIGHT_CANARY;
    }

    return err;
}

// ---------- Dump ----------
void StackDump (struct stack_t* stk, int err
               ON_DBG(, const char* file, int line, const char* func))

{
    printf ("\n========== STACK DUMP ==========\n");

#ifdef ONDEBUG
    printf("Created by  <%s> in Line <%d>   in <%s> func\n", file, line, func);
#endif

    if (stk == NULL)
    {
        printf ("stk == NULL\n");
        printf ("================================\n\n");
        return;
    }

#ifdef ONDEBUG
    if (stk->name == NULL)
        printf ("name       = <unnamed>\n");
    else
        printf ("name       = %s\n", stk->name);
#endif

    printf ("stk        = %p\n", (void*)stk);
    printf ("data       = %p\n", (void*)stk->data);
    printf ("info_data  = %p\n", (void*)stk->info_data);
    printf ("size       = %zu\n", stk->size);
    printf ("capacity   = %zu\n", stk->capacity);

    PrintfCanary (stk);

    PrintfStackElem (stk);

    PrintfErr (err, stk);
}

// ---------- Resize ----------
void StackResize (struct stack_t* stk, size_t new_capacity, int* err)
{
    assert (stk != NULL);
    assert (err != NULL);

    if (new_capacity == 0)
    {
        *err = STACK_BAD_CAPACITY;
        return;
    }

    size_t bytes = (new_capacity + 2) * sizeof(stack_elem_t);
    stack_elem_t* new_data = realloc (stk->data, bytes);
    if (new_data == NULL)
    {
        *err = STACK_MEMORY_ERROR;
        return;
    }

    stk->data      = new_data;
    stk->info_data = new_data + 1;

    // Левая канарейка — на месте
    stk->data[0] = CANARY_VALUE;

    // новое место для правой канарейки
    stk->data[new_capacity + 1] = CANARY_VALUE;

    if (new_capacity > stk->capacity)
    {
        for (size_t i = stk->capacity; i < new_capacity; ++i)
            stk->info_data[i] = STACK_POISON;
    }

    stk->capacity = new_capacity;
    *err = STACK_OK;
}

// ---------- Push ----------
void StackPush (struct stack_t* stk, stack_elem_t value, int* err)
{
    assert (stk != NULL);
    assert (err != NULL);

    if (stk->size == stk->capacity)
    {
        StackResize (stk, stk->capacity * 2, err);

        if (*err != STACK_OK)
            return;
    }

    stk->info_data[stk->size++] = value;
    *err = STACK_OK;
}

// ---------- Pop ----------
stack_elem_t StackPop (struct stack_t* stk, int* err)
{
    assert (stk != NULL);
    assert (err != NULL);

    if (stk->size == 0)
    {
        *err = STACK_UNDERFLOW;
        return STACK_POISON;
    }

    stack_elem_t value = stk->info_data[--stk->size];

    stk->info_data[stk->size] = STACK_POISON;

    if (stk->capacity > 4 && stk->size * 4 <= stk->capacity)
    {
        StackResize(stk, stk->capacity / 2, err);

        if (*err != STACK_OK)
            return value;
    }

    *err = STACK_OK;
    return value;
}

// ---------- Destroy ----------
void StackDestroy (struct stack_t* stk)
{
    if (stk == NULL)
        return;

    free (stk->data);
    stk->data      = NULL;
    stk->info_data = NULL;
    stk->size      = 0;
    stk->capacity  = 0;

#ifdef ONDEBUG
    stk->name = NULL;
    stk->file = NULL;
    stk->line = 0;
    stk->func = NULL;
#endif
}

void PrintfErr (int err, struct stack_t* stk) // НОВУЮ ФУНКЦИЮ для вывода ошибок (-30 строк кода) -- achieved
{
    int verify = StackVerify(stk); 
    printf ("Verify err = %d ", verify);

    if (verify == STACK_OK) 
    {
        printf("(OK)\n");
    }
    
    else
    {
        if (verify & STACK_NULL_PTR)     
            printf ("NULL_PTR ");

        if (verify & STACK_MEMORY_ERROR) 
            printf ("MEMORY ");

        if (verify & STACK_UNDERFLOW)    
            printf ("UNDERFLOW ");

        if (verify & STACK_OVERFLOW)     
            printf ("OVERFLOW ");

        if (verify & STACK_BAD_CAPACITY) 
            printf ("BAD_CAPACITY ");

        if (verify & STACK_LEFT_CANARY)  
            printf ("LEFT_CANARY ");

        if (verify & STACK_RIGHT_CANARY) 
            printf ("RIGHT_CANARY ");

        if (verify & STACK_DATA_NULL)    
            printf ("DATA_NULL ");

        printf("\n");
    }

    if (err == 0)
        printf ("Passed err = %d (%s)\n", err, "OK");
    
    else
        printf ("Passed err = %d \n", err);
    
    printf ("================================\n\n");
}

void PrintfStackElem ( struct stack_t* stk)
{
    if (stk->data != NULL && stk->info_data != NULL)
    {
        for (size_t i = 0; i < stk->capacity; ++i)
        {
            if (i >= stk->size)
            {
                if ( isnan(stk->info_data[i]) != 0 )
                {
                    printf ("[%zu] = " STACK_ELEM "%s\n", i, stk->info_data[i]," <!poison!>");
                }
                else
                {
                    printf ("[%zu] = " STACK_ELEM "%s\n", i, stk->info_data[i]," <!NO poison!>");
                }
            }
            else
            {
                printf ("[%zu] = " STACK_ELEM "\n", i, stk->info_data[i]);
            }
        }
    }
    else
    {
        printf (" <data is NULL>\n");
    }
}

void PrintfCanary (struct stack_t* stk)
{
    if (stk->data != NULL) // новую функцию
    {
        stack_elem_t left_canary  = stk->data[0];
        stack_elem_t right_canary = stk->data[stk->capacity + 1];

        if ( left_canary == CANARY_VALUE)
        {
            printf ("left  canary = " STACK_ELEM " %s\n", left_canary, "<OK>"); 
        }
        else
        {
            printf ("left  canary = " STACK_ELEM " %s\n", left_canary,"<BAD>"); 
        }


        if ( right_canary == CANARY_VALUE)
        {
            printf ("right  canary = " STACK_ELEM " %s\n", right_canary,"<OK>"); 
        }
        else
        {
            printf ("right  canary = " STACK_ELEM " %s\n", right_canary,"<BAD>"); //
        }
    }
}