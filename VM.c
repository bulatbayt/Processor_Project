#include "stack_v3.h"
#include <sys/stat.h>
 
/*todo
поменять привязку к double -- achieved
сделать Dump в дебаг -- achieved
arc, argv для перенаправления ввода вывода
*/

#define SIZE_BUF 256

struct File_info
{
    int max_rows;
    int long long size_file;

};

struct processor_info
{
stack_elem_t registers[4];
size_t pc;
stack_elem_t* code;
size_t code_size;
};

enum Opcode
{
    OP_HLT  = 0,
    OP_PUSH = 1,
    OP_ADD  = 2,
    OP_SUB  = 3,
    OP_DIV  = 4,
    OP_OUT  = 5,
    OP_MULT = 8,
};

int ReadFromFile (const char* name_file, char* index[], char** buffer, struct File_info* Command);
int realise_comand (struct stack_t* stk, int* err, char* index[], int max_rows, struct processor_info* prc1);


void Add (struct stack_t* stk, int* err);
void Sub (struct stack_t* stk, int* err);
void Div (struct stack_t* stk, int* err);
void Out (struct stack_t* stk, int* err);


int main (int argc, char* argv[]) 
{
    struct stack_t stk1 = {};
    struct File_info Command_info = { };
    struct processor_info prc1 = { };

    char* index[123];
    char* buffer;

    double code_array[1000] = { };
    prc1.code = code_array;

    int err = 0;

    STACK_INIT (&stk1, 2, &err);

    STACK_DUMP (&stk1, err);

    int max_rows = ReadFromFile ("Processor_write.asm", index, &buffer, &Command_info);

    realise_comand (&stk1, &err, index, max_rows, &prc1); // обрабатывать return

    printf ("Code\n");
    for (int i = 0; i < prc1.pc; i++)
    {
        printf ("%lg ", prc1.code[i]);
    }

    /*printf ("Check : %lg\n", prc1.code[1]);

    printf ("BUFFFER:\n");
    printf ("%s\n", index[0]);*/

    free (buffer);

    return 0;
}

// ============================================================
// Реализация команд
// ============================================================

int realise_comand (struct stack_t* stk, int* err, char* index[], int max_rows, struct processor_info* prc1) 
{
    /*FILE* fp = fopen ("Processor_write.asm", "r"); 
    if (fp == NULL)
    {
        printf ("Cannot open code.asm\n"); 
        return 1;
    }*/


    
    int command_code = 0;

    for (int i = 0; i < max_rows; i++) 
    {
        stack_elem_t value = 0; 

        sscanf (index[i], "%d %lg", &command_code, &value); 
         
        *(prc1->code + prc1->pc) = (double)command_code;

        switch (command_code)
        {
            case OP_PUSH:
            {   
                StackPush (stk, value, err);
                *(prc1->code + prc1->pc +1) = value;
                prc1->pc = prc1->pc + 2;
                break;
            }

            case OP_ADD:
            {
                Add (stk, err);
                prc1->pc ++;
                break;
            }

            case OP_SUB:
            {
                Sub (stk, err);
                prc1->pc ++;
                break;
            }

            case OP_DIV:
            {
                Div (stk, err);
                prc1->pc ++;
                break;
            }

            case OP_OUT:
            {
                Out (stk, err);
                prc1->pc ++;
                break;
            }


            case OP_HLT:
            {
                StackDestroy (stk);
                //fclose(fp);
                prc1->pc ++;
                //printf ("Succecc completed command = %zu\n", pc);
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

    //fclose (fp);
    return 0;

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

void Mult (struct stack_t* stk, int* err)
{
    assert (stk != NULL);
    assert (err != NULL);

    stack_elem_t Val2 = StackPop (stk, err);
    if (*err != STACK_OK) 
    { 
        printf ("Multiplication: not enough operands\n"); 
        return; 
    }

    stack_elem_t Val1 = StackPop (stk, err);
    if (*err != STACK_OK) 
    { 
        printf ("Multiplication: not enough operands\n"); 
        return; 
    }


    stack_elem_t multiplication = Val1 * Val2;

    printf ("Multiplication <"STACK_ELEM">\n", multiplication);
    StackPush (stk, multiplication, err);

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

int ReadFromFile (const char* name_file, char* index[], char** buffer, struct File_info* Command)
{
    assert (name_file != NULL);
    assert (index != NULL);

    FILE* Command_file = fopen (name_file, "rb");

    int file_dis = fileno (Command_file);

    if (Command_file == NULL)
    {
        perror ("fopen"); 
        return 0;
    }

    struct stat stat_file;

    fstat (file_dis, &stat_file);

    Command->size_file = stat_file.st_size;

    // TODO: use arrows
    *buffer = calloc(Command->size_file + 1, sizeof(char));

    fread(*buffer, sizeof(char), Command->size_file, Command_file);
    (*buffer)[Command->size_file] = '\0';

    int max_rows = 0; 

    index [0] = *buffer; 

    for (int i = 0; i < Command->size_file; i ++)
    {
        if ( *(*buffer + i) == '\n')
        {
            //*(*buffer + i) = '\0';       
            max_rows ++;
            index [ max_rows] =  *buffer + i + 1;
        }
    }
    
    fclose (Command_file);

    return max_rows;
}
