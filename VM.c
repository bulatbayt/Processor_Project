#include "stack_v3.h"
#include <string.h>

/*todo
поменять привязку к double
*/

void Add (struct stack_t* stk, int* err);
void Sub (struct stack_t* stk, int* err);
void Div (struct stack_t* stk, int* err);
void Out (struct stack_t* stk, int* err);

//int ExecuteCommand (struct stack_t* stk, const char* line, int* err);



int main ()
{
    struct stack_t stk1 = {};
    int err = 0;

    STACK_INIT (&stk1, 2, &err);

    FILE* fp = fopen ("processor_write.asm", "r"); // новую функцию
    if (fp == NULL)
    {
        printf ("Cannot open code.asm\n");
        return 1;
    }

    char* command_string = calloc (32, sizeof(char));
    int command_code = 0;
     

    while (fgets (command_string, 32-1, fp) != NULL) // новая функция 
    {
        sscanf (command_string, "%d", &command_code);
     
        double value = 0; //привязка к double

        //printf ("Command code :%d\n", command_code);

        switch (command_code)
        {
            case 1:
            {
                sscanf (command_string, "%d %lg",&command_code, &value);
                //printf ("Value : %lg\n", value);
                StackPush (&stk1, value, &err);
                break;
            }

            case 2:
            {
                Add (&stk1, &err);
                break;
            }

            case 3:
            {
                Sub (&stk1, &err);
                break;
            }

            case 4:
            {
                Div (&stk1, &err);
                break;
            }

            case 5:
            {
                printf ("Results: %lg\n", StackPop(&stk1, &err));
                break;
            }


            case 0:
            {
                StackDestroy (&stk1);
                break;
            }

            default:
            {
                printf ("Uncnown command\n");
                break;
            }
        }
    }

    /*while (fgets (line, sizeof (line), fp) != NULL)
    {
        int result = ExecuteCommand (&stk1, line, &err);

        if (result == 0) 
        break;   // HLT

        if (err != STACK_OK)
        {
            printf ("Runtime error: %d\n", err);
            STACK_DUMP (&stk1, err);
            break;
        }
    }*/

    fclose (fp);

    //printf ("\nStack:\n");
    //STACK_DUMP (&stk1, err);

    return 0;
}

// ============================================================
// Команды
// ============================================================
void Add (struct stack_t* stk, int* err)
{
    assert (stk != NULL);
    assert (err != NULL);

    double Val2 = StackPop (stk, err);
    if (*err != STACK_OK) 
    { 
        printf ("ADD: not enough operands\n"); 
        return; 
    }

    double Val1 = StackPop (stk, err);
    if (*err != STACK_OK) 
    { 
        printf ("ADD: not enough operands\n"); 
        return; 
    }

    double sum = Val1 + Val2;

    printf ("sum <%lg>\n", sum);
    StackPush (stk, sum, err);

    *err = STACK_OK;
}

void Sub (struct stack_t* stk, int* err)
{
    assert (stk != NULL);
    assert (err != NULL);

    double Val2 = StackPop (stk, err);
    if (*err != STACK_OK) 
    { 
        printf ("SUB: not enough operands\n"); return; 
    }

    double Val1 = StackPop (stk, err);
    if (*err != STACK_OK) 
    { 
        printf ("SUB: not enough operands\n"); return; 
    }

    double difference = Val1 - Val2;

    printf ("difference <%lg>\n", difference);
    StackPush (stk, difference, err);

    *err = STACK_OK;
}

void Div (struct stack_t* stk, int* err)
{
    assert (stk != NULL);
    assert (err != NULL);

    double Val2 = StackPop (stk, err);
    if (*err != STACK_OK) 
    { 
        printf ("DIV: not enough operands\n"); 
        return; 
    }

    double Val1 = StackPop (stk, err);
    if (*err != STACK_OK) 
    { 
        printf ("DIV: not enough operands\n"); 
        return; 
    }

    if (Val2 == 0.0)
    {
        printf ("DIV: division by zero\n");
        *err = STACK_OK;
        return;
    }

    double quotient = Val1 / Val2;

    printf ("quotient <%lg>\n", quotient);
    StackPush (stk, quotient, err);

    *err = STACK_OK;
}

void Out (struct stack_t* stk, int* err)
{
    double v = StackPop (stk, err);
    if (*err != STACK_OK) 
    { 
        printf ("OUT: stack is empty\n"); 
        return; 
    }

    printf ("Out value: %lg\n", v);
    *err = STACK_OK;
}