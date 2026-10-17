#include "stack_v3.h"
#include <sys/stat.h>

/*todo
поменять привязку
arc, argv для перенаправления ввода вывода

fread, fwrite -- бинарный тип , переход

Ассемблер добавить CALL и рет

таблица меток, глобальная переменная чтоли?
*/

#define SIZE_BUF        256
#define LABEL_MAX       4
#define REG_COUNT       6
#define ROWS_FOR_INDEX  123

struct Processor_Command
{
    int             max_rows;
    int long long   size_file;
    char*           index[ROWS_FOR_INDEX];   
    char*           buffer;
    int             opcode;
    double          value;
};

enum Opcode
{
    OP_HLT      = 0,
    OP_PUSH     = 1,
    OP_ADD      = 2,
    OP_SUB      = 3,
    OP_DIV      = 4,
    OP_OUT      = 5,
    OP_POP      = 6,
    OP_JMP      = 7,
    OP_MUL      = 8,
    OP_JA       = 9,
    OP_PUSHr    = 10,
    OP_CALL     = 11,
    OP_RET      = 12,
};

struct processor_info
{
    stack_elem_t registers[REG_COUNT]; // Ax Bx Cx CallX ---- как 
    size_t       ip;                    
    size_t       code_size;
    int          labels[LABEL_MAX];  
    size_t       labels_amount;
    int          err;
    stack_elem_t code[1000]; // в константу или динамика
};


//=====================================================================================================================//
//=============================================Прототипы Вспомогательных функций ======================================//
//=====================================================================================================================//

void Label_Init (struct processor_info* prc);

int  ReadFromFile         (const char* read_file, struct Processor_Command* Command);

int  realise_comand       (struct stack_t* stk, struct Processor_Command Command,
                          struct processor_info* prc1);

int  ProcessorVerify      (struct processor_info* prc1, struct stack_t* stk, int* err);
void CPU_Dump             (struct processor_info* prc1, FILE* write_CDump);
void Parse_Comand         (struct Processor_Command* Command, struct processor_info* prc1);
int  Start_Commands       (struct processor_info* prc1, FILE* write_CDump, int* err, struct stack_t* stk);

//=====================================================================================================================//
//=============================================Прототипы Команд Процессора=============================================//
//=====================================================================================================================//

void Add       (struct stack_t* stk, struct processor_info* prc1);
void Sub       (struct stack_t* stk, struct processor_info* prc1);
void Div       (struct stack_t* stk, struct processor_info* prc1);
void Mult      (struct stack_t* stk, struct processor_info* prc1);
void Out       (struct stack_t* stk, struct processor_info* prc1);
int  Jump      (struct processor_info* prc1);
int Jump_Above (struct stack_t* stk, struct processor_info* prc1);
int Call       (struct stack_t* stk, struct processor_info* prc1);
int Ret        (struct stack_t* stk, struct processor_info* prc1);
void Hlt       (struct processor_info* prc1);

//=====================================================================================================================//

int main (int argc, char* argv[]) // !!!!!!!!!!! юзаем
{
    struct stack_t stk1 = {};
    struct Processor_Command Command_info = {};
    struct processor_info prc1 = {};
    struct stack_t stk2 = {};


    Label_Init (&prc1);
    STACK_INIT (&stk1, 2, &prc1.err);// Передавать только структуры в параметры
    STACK_INIT (&stk2, 2, &prc1.err); 

    printf (BLUE"\n========================Processing working========================\n"RESET_COLOR);

    ReadFromFile ("Processor_write.asm", &Command_info);

    realise_comand (&stk1, Command_info, &prc1);

    StackDestroy (&stk1, &prc1.err);
    StackDestroy (&stk2, &prc1.err);
    
    free (Command_info.buffer);

    return 0;
}

// ============================================================
// Вспомогательные функции
// ============================================================

void Label_Init (struct processor_info* prc)
{
    for (size_t i = 0; i < LABEL_MAX; i++) 
    {
        prc->labels[i] = -1;
    }
}

int realise_comand (struct stack_t* stk, struct Processor_Command Command, struct processor_info* prc1)
{
    
    Parse_Comand (&Command, prc1);

    int verify = ProcessorVerify (prc1, stk, &prc1->err);
    if (verify != 0)
    {
        PrintfErr (prc1->err, stk);
        return -1;
    }

    FILE* write_CDump = fopen ("Cpu_Dump.txt", "w");
    if (write_CDump == NULL)
    {
        printf ("Error with open file");
        return -1;
    }

    if ( Start_Commands (prc1, write_CDump, &prc1->err, stk) != 0)
    {
        printf ("Error witg realise command, reviev the errors\n");
        return -1;
    }

    fclose (write_CDump);

    return 0;
}

int ProcessorVerify (struct processor_info* prc1, struct stack_t* stk1, int* err) 
{
    *err = StackVerify (stk1);
    if (prc1 == NULL) 
        *err |= PRC_NULL;                
    if (prc1->code == NULL)    
        *err |= PRC_CODE_NULL;
    if (prc1->ip > prc1->code_size) 
        *err |= PRC_BAD_IP;

    
    if (*err != STACK_OK)              
        return 1;

    return 0;
}

void CPU_Dump (struct processor_info* prc1, FILE* write_CDump) 
{
    #ifdef ONDEBUG
    fprintf (write_CDump, "\n----- CODE -----\n");
    for (size_t i = 0; i < prc1->code_size; i++)
    {
        fprintf (write_CDump, "  [%zu] " STACK_ELEM , i, prc1->code[i]);
        
        if (prc1->ip == i)
        {
            fprintf (write_CDump, "    [<---------");
        } 
        fprintf (write_CDump, "\n");
    }

    /*printf ("\n----- LABELS -----\n"); // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! починить
    for (int i = 0; i < LABEL_MAX; i++)
    {
        if (prc1->labels[i] != -1)
            printf ("  :%d -> %d\n", i, prc1->labels[i]);
    }*/
    #endif
}

int Start_Commands (struct processor_info* prc1, FILE* write_CDump, int* err, struct stack_t* stk) 
{
    while (prc1->ip < prc1->code_size) 
    {
        int command_code = (int)prc1->code[prc1->ip];
        CPU_Dump (prc1, write_CDump);
        switch (command_code)
        {
            case OP_PUSH:
            {
                stack_elem_t value = prc1->code[prc1->ip + 1];
                StackPush (stk, value, err);
                prc1->ip += 2;
                break;
            }

            case OP_PUSHr:
            {
                int reg = (int)prc1->code[prc1->ip + 1];
                StackPush (stk, prc1->registers[reg], err);
                prc1->ip += 2;
                break;
            }

            case OP_POP: 
            {
                int reg = (int)prc1->code[prc1->ip + 1];
                stack_elem_t v = StackPop (stk, err);
                if (*err != STACK_OK)
                {
                    printf ("POP: stack is empty\n");
                    return -1;
                }
                prc1->registers[reg] = v;
                prc1->ip += 2;
                break;
            }

            case OP_ADD:
            { 
                Add (stk, prc1); 
                prc1->ip++; 
                break;
            }

            case OP_SUB:
            {
                Sub (stk, prc1); 
                prc1->ip++; 
                break;
            }

            case OP_DIV:
            {
                Div (stk, prc1); 
                prc1->ip++; 
                break;
            }

            case OP_MUL:
            {
                Mult (stk, prc1); 
                prc1->ip++; 
                break;
            }

            case OP_OUT: 
            {
                Out (stk, prc1); 
                prc1->ip++; 
                break;
            }

            case OP_JMP: //!!!!!!!!!!!!! функцию --- achived
            {
                if (Jump (prc1) == -1)
                {
                    fclose (write_CDump);
                    return -1;
                }
                break;
            }

            case OP_JA: // !!!!!!!!!!!!!!!! функцию -- achieved
            {
                if ( Jump_Above(stk, prc1) == -1)
                {
                    fclose (write_CDump);
                    return -1;
                }
                break;
            }

            case OP_HLT:
            {
                prc1->ip++;
                Hlt (prc1);
                fclose (write_CDump);
                return 0;
            }

            default:
            {
                printf ("Uncnown command\n");
                fclose (write_CDump);
                return -1;
            }
        }

        if (*err != STACK_OK)
        {
            printf ("Runtime error: %d at IP=%zu\n", *err, prc1->ip);
            fclose (write_CDump);
            return -1;
        }
    }
    return 0;
}

int ReadFromFile (const char* read_file, struct Processor_Command* Command)
{
    assert (read_file != NULL);
    assert (Command->index != NULL);

    FILE* Command_file = fopen (read_file, "rb");

    if (Command_file == NULL)
    {
        perror ("fopen");
        return -1;
    }

    int file_dis = fileno (Command_file);

    struct stat stat_file;

    fstat (file_dis, &stat_file);

    Command->size_file = stat_file.st_size;

    Command->buffer = calloc (Command->size_file + 1, sizeof (char));
    if (Command->buffer == NULL)
    {
        fclose (Command_file);
        return -1;
    }

    fread (Command->buffer, sizeof (char), Command->size_file, Command_file);
    (Command->buffer)[Command->size_file] = '\0';

    Command->index [0] = Command->buffer;

    for (int i = 0; i < Command->size_file; i ++)
    {
        if ( *(Command->buffer + i) == '\n')
        {
            *(Command->buffer + i) = '\0';
            Command->max_rows ++;
            Command->index [ Command->max_rows] =  Command->buffer + i + 1;
        }
    }

    fclose (Command_file);

    return 0;
}

void Parse_Comand (struct Processor_Command* Command, struct processor_info* prc1)
{
    for (int i = 0; i < Command->max_rows; i++)
    {
        

        if (sscanf (Command->index[i], "%d %lf", &Command->opcode, &Command->value) == 2)
        {
            prc1->code[prc1->code_size++] = (stack_elem_t)Command->opcode;
            prc1->code[prc1->code_size++] = (stack_elem_t)Command->value;
        }
        else if (sscanf (Command->index[i], "%d", &Command->opcode) == 1)
        {
            prc1->code[prc1->code_size++] = (stack_elem_t)Command->opcode;
        }       
    }
}

// ============================================================
// Команды Процессора
// ============================================================
void Add (struct stack_t* stk, struct processor_info* prc1)
{
    assert (stk != NULL);
    assert (&prc1->err != NULL);

    stack_elem_t Val2 = StackPop (stk, &prc1->err);
    if (prc1->err != STACK_OK)
    {
        printf ("ADD: not enough operands\n");
        return;
    }

    stack_elem_t Val1 = StackPop (stk, &prc1->err);
    if (prc1->err != STACK_OK)
    {
        printf ("ADD: not enough operands\n");
        return;
    }

    stack_elem_t sum = Val1 + Val2;

    //printf ("sum <"STACK_ELEM">\n", sum);
    StackPush (stk, sum, &prc1->err);

    prc1->err = STACK_OK;
}

void Sub (struct stack_t* stk, struct processor_info* prc1)
{
    assert (stk != NULL);
    assert (&prc1->err != NULL);

    stack_elem_t Val2 = StackPop (stk, &prc1->err);
    if (prc1->err != STACK_OK)
    {
        printf ("SUB: not enough operands\n");
        return;
    }

    stack_elem_t Val1 = StackPop (stk, &prc1->err);
    if (prc1->err != STACK_OK)
    {
        printf ("SUB: not enough operands\n");
        return;
    }

    stack_elem_t difference = Val1 - Val2;

    //printf ("difference <"STACK_ELEM">\n", difference);
    StackPush (stk, difference, &prc1->err);

    prc1->err = STACK_OK;
}

void Div (struct stack_t* stk, struct processor_info* prc1)
{
    assert (stk != NULL);
    assert (&prc1->err != NULL);

    stack_elem_t Val2 = StackPop (stk, &prc1->err);
    if (prc1->err != STACK_OK)
    {
        printf ("DIV: not enough operands\n");
        return;
    }

    stack_elem_t Val1 = StackPop (stk, &prc1->err);
    if (prc1->err != STACK_OK)
    {
        printf ("DIV: not enough operands\n");
        return;
    }

    if (Val2 == 0)
    {
        printf ("DIV: division by zero\n");
        prc1->err = STACK_OK;
        return;
    }

    stack_elem_t quotient = Val1 / Val2;

    //printf ("quotient <"STACK_ELEM">\n", quotient);
    StackPush (stk, quotient, &prc1->err);

    prc1->err = STACK_OK;
}

void Mult (struct stack_t* stk, struct processor_info* prc1)
{
    assert (stk != NULL);
    assert (&prc1->err != NULL);

    stack_elem_t Val2 = StackPop (stk, &prc1->err);
    if (prc1->err != STACK_OK)
    {
        printf ("Multiplication: not enough operands\n");
        return;
    }

    stack_elem_t Val1 = StackPop (stk, &prc1->err);
    if (prc1->err != STACK_OK)
    {
        printf ("Multiplication: not enough operands\n");
        return;
    }

    stack_elem_t multiplication = Val1 * Val2;

    //printf ("Multiplication <"STACK_ELEM">\n", multiplication);
    StackPush (stk, multiplication, &prc1->err);

    prc1->err = STACK_OK;
}

void Out (struct stack_t* stk, struct processor_info* prc1)
{
    stack_elem_t out_elem = StackPop (stk, &prc1->err); //!!!!!!!!!!!!!!!!!!!!!!!!!!название переменной 
    if (prc1->err != STACK_OK)
    {
        printf ("OUT: stack is empty\n");
        return;
    }
    printf ("OP_OUT: ");
    //printf ("Out value: "STACK_ELEM"\n", out_elem;
    printf (STACK_ELEM"\n", out_elem);
    prc1->err = STACK_OK;
}

int  Jump (struct processor_info* prc1)
{
    int aim = (int)prc1->code[prc1->ip + 1];
    /*if ( prc1->labels_amount < LABEL_MAX)
    {
    prc1->labels[prc1->labels_amount++] = aim; //пока просто записываем метки -- так метки не запишем
    }*/

    if (aim < 0 || (size_t)aim >= prc1->code_size)
    {
        printf ("JMP fail: bad address %d\n", aim);
        return -1;
    }
    prc1->ip = (size_t)aim;
    return 0;
}

int Jump_Above (struct stack_t* stk, struct processor_info* prc1)
{
    int aim = (int)prc1->code[prc1->ip + 1];
    /*if ( prc1->labels_amount < LABEL_MAX)
    {
        prc1->labels[prc1->labels_amount++] = aim; //пока просто записываем метки -- так метки не запишем
    }*/

    if (aim < 0 || (size_t)aim >= prc1->code_size)
    {
        printf ("JA fail: bad address %d\n", aim);
        return -1;
    }

    stack_elem_t a = StackPop (stk, &prc1->err);
    stack_elem_t b = StackPop (stk, &prc1->err);
    if (prc1->err != STACK_OK)
    {
        printf ("JA: stack underflow\n");
        return -1;
    }

    if (a > b) prc1->ip = (size_t)aim;
    else       prc1->ip += 2;

    return 0;
}

int Call (struct stack_t* stk, struct processor_info* prc1)
{
    int aim = (int)prc1->code[prc1->ip + 1];
    
    if (aim < 0 || (size_t)aim >= prc1->code_size)
    {
        printf ("Call fail: bad address %d\n", aim);
        return -1;
    }

    stack_elem_t push_value = (stack_elem_t)(prc1->ip + 2);   // сделать стэк для возврата!

    StackPush (stk, push_value, &prc1->err);

    prc1->ip = (size_t)aim;
    return 0;
}

int Ret (struct stack_t* stk, struct processor_info* prc1)
{
    prc1->ip = StackPop (stk, &prc1->err);
    return 0;
}

void Hlt (struct processor_info* prc1)
{

    printf     (BLUE"\n\n===============================Processor finish work===============================\n"RESET_COLOR);
    printf     ("HLT at IP=%zu\n", prc1->ip);
    printf     (BLUE    "===================================================================================\n"RESET_COLOR);
}

