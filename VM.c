#include "stack_v3.h"

/*todo
поменять привязку к double -- achieved
сделать Dump в дебаг
arc, argv для перенаправления ввода вывода
*/

#define SIZE_BUF 256

int realise_comand (struct stack_t* stk, int* err);

void Add (struct stack_t* stk, int* err);
void Sub (struct stack_t* stk, int* err);
void Div (struct stack_t* stk, int* err);
void Out (struct stack_t* stk, int* err);


int main () // ARGC ARGV
{
    struct stack_t stk1 = {};
    int err = 0;

    STACK_INIT (&stk1, 2, &err);

    STACK_DUMP (&stk1, err);

    realise_comand (&stk1, &err); // обрабатывать return

    return 0;
}

// ============================================================
// Реализация команд
// ============================================================

int realise_comand (struct stack_t* stk, int* err) 
{
    FILE* fp = fopen ("Processor_write.asm", "r"); 
    if (fp == NULL)
    {
        printf ("Cannot open code.asm\n"); 
        return 1;
    }

    char* command_string = calloc (SIZE_BUF, sizeof(char));
    int command_code = 0;


    while (fgets (command_string, SIZE_BUF-1, fp) != NULL) 
    {
        stack_elem_t value = 0; //привязка к double

        sscanf (command_string, "%d "STACK_ELEM,&command_code, &value);

        switch (command_code)
        {
            case 1:
            {   
                StackPush (stk, value, err);
                break;
            }

            case 2:
            {
                Add (stk, err);
                break;
            }

            case 3:
            {
                Sub (stk, err);
                break;
            }

            case 4:
            {
                Div (stk, err);
                break;
            }

            case 5:
            {
                Out (stk, err);
                break;
            }


            case 0:
            {
                StackDestroy (stk);
                fclose(fp);
                free(command_string);
                return 0;
                break;
            }

            default:
            {
                printf ("Uncnown command\n");
                break;
            }
        }
    }



    free (command_string);
    fclose (fp);

}

// ============================================================
// Команды
// ============================================================
void Add (struct stack_t* stk, int* err)
{
    assert (stk != NULL);
    assert (err != NULL);

    stack_elem_t Val2 = StackPop (stk, err);
    if (*err != STACK_OK) 
    { 
        printf ("ADD: not enough operands\n"); 
        return; 
    }

    stack_elem_t Val1 = StackPop (stk, err);
    if (*err != STACK_OK) 
    { 
        printf ("ADD: not enough operands\n"); 
        return; 
    }

    stack_elem_t sum = Val1 + Val2;

    printf ("sum <"STACK_ELEM">\n", sum);
    StackPush (stk, sum, err);

    *err = STACK_OK;
}

void Sub (struct stack_t* stk, int* err)
{
    assert (stk != NULL);
    assert (err != NULL);

    stack_elem_t Val2 = StackPop (stk, err);
    if (*err != STACK_OK) 
    { 
        printf ("SUB: not enough operands\n"); return; 
    }

    stack_elem_t Val1 = StackPop (stk, err);
    if (*err != STACK_OK) 
    { 
        printf ("SUB: not enough operands\n"); return; 
    }

    stack_elem_t difference = Val1 - Val2;

    printf ("difference <"STACK_ELEM">\n", difference);
    StackPush (stk, difference, err);

    *err = STACK_OK;
}

void Div (struct stack_t* stk, int* err)
{
    assert (stk != NULL);
    assert (err != NULL);

    stack_elem_t Val2 = StackPop (stk, err);
    if (*err != STACK_OK) 
    { 
        printf ("DIV: not enough operands\n"); 
        return; 
    }

    stack_elem_t Val1 = StackPop (stk, err);
    if (*err != STACK_OK) 
    { 
        printf ("DIV: not enough operands\n"); 
        return; 
    }

    if (Val2 == 0)
    {
        printf ("DIV: division by zero\n");
        *err = STACK_OK;
        return;
    }

    stack_elem_t quotient = Val1 / Val2;

    printf ("quotient <"STACK_ELEM">\n", quotient);
    StackPush (stk, quotient, err);

    *err = STACK_OK;
}

void Out (struct stack_t* stk, int* err)
{
    stack_elem_t v = StackPop (stk, err);
    if (*err != STACK_OK) 
    { 
        printf ("OUT: stack is empty\n"); 
        return; 
    }

    printf ("Out value: "STACK_ELEM"\n", v);
    *err = STACK_OK;
}
