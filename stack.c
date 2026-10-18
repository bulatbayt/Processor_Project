#include "stack_v3.h"


//============================================================================
//===========================Управление стэком================================
//============================================================================

void StackInit (struct stack_t* stk, size_t capacity, int* err, ...)
{
    assert (stk != NULL);
    assert (err != NULL);

    va_list args;
    va_start (args, err);

#ifdef ONDEBUG
    stk->name = va_arg (args, const char*);
    stk->file = va_arg (args, const char*);
    stk->line = va_arg (args, int);
    stk->func = va_arg (args, const char*);
#endif

    va_end (args);

    if (capacity == 0)
    {
        *err = STACK_BAD_CAPACITY;
        return;
    }

    stk->data = calloc ((capacity + 2) , sizeof (stack_elem_t));

    if (stk->data == NULL)
    {
        stk->capacity = 0;
        *err = STACK_MEMORY_ERROR;
        return;
    }

    stk->data[0] = CANARY_VALUE;
    stk->info_data = stk->data + 1;
    stk->data[capacity + 1] = CANARY_VALUE;

    for (size_t i = 0; i < capacity; ++i)
    {
        stk->info_data[i] = STACK_POISON;
    }
    
    stk->size     = 0;
    stk->capacity = capacity;
    *err = STACK_OK;

    STACK_DUMP (stk, *err);
}

int StackVerify (struct stack_t* stk)
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

void StackResize (struct stack_t* stk, size_t new_capacity, int* err)
{
    assert (stk != NULL);
    assert (err != NULL);

    if (new_capacity == 0)
    {
        *err = STACK_BAD_CAPACITY;
        return;
    }

    size_t bytes = (new_capacity + 2) * sizeof (stack_elem_t);
    stack_elem_t* new_data = realloc (stk->data, bytes);
    if (new_data == NULL)
    {
        *err = STACK_MEMORY_ERROR;
        return;
    }

    stk->data      = new_data;
    stk->info_data = new_data + 1;

    stk->data[0] = CANARY_VALUE;
    stk->data[new_capacity + 1] = CANARY_VALUE;

    if (new_capacity > stk->capacity)
    {
        for (size_t i = stk->capacity; i < new_capacity; ++i)
            stk->info_data[i] = STACK_POISON;
    }

    stk->capacity = new_capacity;
    *err = STACK_OK;
}

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
        StackResize (stk, stk->capacity / 2, err);
        if (*err != STACK_OK)
            return value;
    }

    *err = STACK_OK;
    return value;
}

void StackDestroy (struct stack_t* stk, int* err)
{
    if (stk == NULL)
        return;

    STACK_DUMP (stk, *err);

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


//============================================================================
//===========================Функции вывода информации========================
//============================================================================
void StackDump (struct stack_t* stk, int err
                ON_DBG(, const char* file, int line, const char* func))
{
#ifdef ONDEBUG
    printf (BLUE"\n========== STACK DUMP ==========\n" RESET_COLOR);

//#ifdef ONDEBUG
    printf ("Created by  <%s> in Line <%d>   in <%s> func\n", file, line, func);
//#endif

    if (stk == NULL)
    {
        printf ("stk == NULL\n");
        printf ("================================\n\n");
        return;
    }

//#ifdef ONDEBUG
    if (stk->name == NULL)
        printf ("name       = <unnamed>\n");
    else
        printf ("name       = "GREEN"%s\n"RESET_COLOR, stk->name);
//#endif

    printf ("stk        = %p\n", (void*)stk);
    printf ("data       = %p\n", (void*)stk->data);
    printf ("info_data  = %p\n", (void*)stk->info_data);
    printf ("size       = "BLUE"%zu\n"RESET_COLOR, stk->size);
    printf ("capacity   = "BLUE"%zu\n"RESET_COLOR, stk->capacity);

    PrintfCanary (stk);
    PrintfStackElem (stk);
    PrintfErr (err, stk);
#endif
}

void PrintfErr (int err, struct stack_t* stk)
{
    int verify = StackVerify (stk);
    printf ("Verify err = %d ", verify);

    if (verify == STACK_OK)
    {
        printf (GREEN"(OK)\n"RESET_COLOR);
    }
    else
    {
        printf (RED"Errors:\n"RESET_COLOR);
        if (verify & STACK_NULL_PTR)     printf ("NULL_PTR \n");
        if (verify & STACK_MEMORY_ERROR) printf ("MEMORY \n");
        if (verify & STACK_UNDERFLOW)    printf ("UNDERFLOW \n");
        if (verify & STACK_OVERFLOW)     printf ("OVERFLOW \n");
        if (verify & STACK_BAD_CAPACITY) printf ("BAD_CAPACITY \n");
        if (verify & STACK_LEFT_CANARY)  printf ("LEFT_CANARY \n");
        if (verify & STACK_RIGHT_CANARY) printf ("RIGHT_CANARY \n");
        if (verify & STACK_DATA_NULL)    printf ("DATA_NULL \n");
        if (verify & PRC_NULL)           printf ("PROCESSOR_STRUCT_NULL\n");
        if (verify & PRC_CODE_NULL)      printf ("PROCESSOR_CODE_NULL\n");
        if (verify & PRC_BAD_IP)         printf ("BAD_IP\n");
        printf ("\n");
        printf ("Debug it )))))))))\n");
    }

    if (err == 0)
        printf ("Passed err = %d (%s)\n", err, GREEN"OK"RESET_COLOR);
    else
        printf ("Passed err = %d \n", err);

    printf (BLUE"================================\n\n"RESET_COLOR);
}

void PrintfStackElem (struct stack_t* stk)
{
    if (stk->data != NULL && stk->info_data != NULL)
    {
        for (size_t i = 0; i < stk->capacity; ++i)
        {
            if (i >= stk->size)
            {
                if (isnan (stk->info_data[i]) != 0)
                    printf ("[%zu] = " STACK_ELEM "%s\n", i, stk->info_data[i], BLUE" <!poison!>"RESET_COLOR);
                else
                    printf ("[%zu] = " STACK_ELEM "%s\n", i, stk->info_data[i], RED" <!NO poison!>"RESET_COLOR);
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
    if (stk->data != NULL)
    {
        stack_elem_t left_canary  = stk->data[0];
        stack_elem_t right_canary = stk->data[stk->capacity + 1];

        if (left_canary == CANARY_VALUE)
            printf ("left  canary = " STACK_ELEM " %s\n", left_canary, GREEN"<OK>"RESET_COLOR);
        else
            printf ("left  canary = " STACK_ELEM " %s\n", left_canary, RED"<BAD>"RESET_COLOR);

        if (right_canary == CANARY_VALUE)
            printf ("right  canary = " STACK_ELEM " %s\n", right_canary, GREEN"<OK>"RESET_COLOR);
        else
            printf ("right  canary = " STACK_ELEM " %s\n", right_canary, RED"<BAD>"RESET_COLOR);
    }
}
