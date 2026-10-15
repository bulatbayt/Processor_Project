#include "stack_v3.h"
#include <sys/stat.h>
 
/*todo
поменять привязку к double -- achieved
сделать Dump в дебаг -- achieved
arc, argv для перенаправления ввода вывода



таблица меток, глобальная переменная чтоли?


if (*err != STACK_OK) ---- под эту штуку можно сделать функцию, по типу ассерта
{
    printf ("POP: stack is empty\n");
    return -1;
} 
*/

#define SIZE_BUF        256
#define LABEL_MAX       4
#define REG_COUNT       6

struct File_info
{
    int           max_rows;
    int long long size_file;
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
    OP_PUSHr = 10,

};

struct processor_info
{
    stack_elem_t registers[REG_COUNT]; // Ax Bx Cx CallX
    size_t       ip;                    
    size_t       code_size;
    int          labels[LABEL_MAX];  
    size_t       labels_amount;
    stack_elem_t code[1000]; // в константу или динамика
};

int ReadFromFile     (const char* read_file, char* index[], char** buffer, struct File_info* Command);
int realise_comand   (struct stack_t* stk, int* err, char* index[], int max_rows,
                      struct processor_info* prc1);

int ProcessorVerify  (struct processor_info* prc1);
void Pump            (struct processor_info* prc1);

void Add  (struct stack_t* stk, int* err);
void Sub  (struct stack_t* stk, int* err);
void Div  (struct stack_t* stk, int* err);
void Mult (struct stack_t* stk, int* err);
void Out  (struct stack_t* stk, int* err);
int  Jump (struct processor_info* prc1, int* err);
int Jump_Above (struct stack_t* stk,struct processor_info* prc1, int* err);

void Hlt (struct processor_info* prc1 , int* err);


int main (int argc, char* argv[]) // !!!!!!!!!!! юзаем
{
    struct stack_t stk1 = {};
    struct File_info Command_info = {};
    struct processor_info prc1 = {};

    char* index[123]; // мб в структуру какую нибудь запихнуть
    char* buffer;

    for (int i = 0; i < LABEL_MAX; i++)
        prc1.labels[i] = -1;

    int err = 0;

    STACK_INIT (&stk1, 2, &err);

    STACK_DUMP (&stk1, err);

    int max_rows = ReadFromFile ("Processor_write.asm", index, &buffer, &Command_info);

    realise_comand (&stk1, &err, index, max_rows, &prc1);

    printf ("\n\n ============================================================\n\n");
    Pump (&prc1);
    printf ("\n\n ============================================================\n\n");

    STACK_DUMP (&stk1, err);

    StackDestroy (&stk1);

    free (buffer);

    return 0;
}

// ============================================================
// Реализация команд
// ============================================================

int realise_comand (struct stack_t* stk, int* err, char* index[], int max_rows, struct processor_info* prc1)
{
    prc1->code_size = 0;

    for (int i = 0; i < max_rows; i++)
    {
        int    opcode = 0; // лишние переменные
        double value  = 0;

        if (sscanf (index[i], "%d %lf", &opcode, &value) == 2)
        {
            prc1->code[prc1->code_size++] = (stack_elem_t)opcode;
            prc1->code[prc1->code_size++] = (stack_elem_t)value;
        }
        else if (sscanf (index[i], "%d", &opcode) == 1)
        {
            prc1->code[prc1->code_size++] = (stack_elem_t)opcode;
        }
        
    }


    int verify = ProcessorVerify (prc1);
    if (verify != 0)
    {
        printf ("Processor verification failed: %d\n", verify);
        return -1;
    }

    //prc1->ip = 0;

    while (prc1->ip < prc1->code_size) // новую функцию???????????????
    {
        int command_code = (int)prc1->code[prc1->ip];

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
                Add (stk, err); 
                prc1->ip++; 
                break;
            }

            case OP_SUB:
            {
                Sub (stk, err); 
                prc1->ip++; 
                break;
            }

            case OP_DIV:
            {
                Div (stk, err); 
                prc1->ip++; 
                break;
            }

            case OP_MUL:
            {
                Mult (stk, err); 
                prc1->ip++; 
                break;
            }

            case OP_OUT: 
            {
                Out (stk, err); 
                prc1->ip++; 
                break;
            }

            case OP_JMP: //!!!!!!!!!!!!! функцию --- achived
            {
                if (Jump (prc1, err) == -1)
                {
                    return -1;
                }
                break;
            }

            case OP_JA: // !!!!!!!!!!!!!!!! функцию
            {
                if ( Jump_Above(stk, prc1, err) == -1)
                {
                    return -1;
                }
                break;
            }

            case OP_HLT:
            {
                prc1->ip++;
                Hlt (prc1, err);
                return 0;
            }

            default:
            {
                printf ("Uncnown command\n");
                return -1;
            }
        }

        if (*err != STACK_OK)
        {
            printf ("Runtime error: %d at IP=%zu\n", *err, prc1->ip);
            return -1;
        }
    }

    return 0;
}


int ProcessorVerify (struct processor_info* prc1) //!!!!!!!!!!!!!!!!!!!! Почему он не вызывает верификатор стэка it musts, короче оптимизируем с verivicatorom stacka -- это не годится
{
    if (prc1 == NULL)                 return -1;
    if (prc1->code == NULL)           return -2;
    if (prc1->ip > prc1->code_size)   return -3;

    

    return 0;
}


void Pump (struct processor_info* prc1) // печатаем код и метки
{
    printf ("\n----- CODE -----\n");
    for (size_t i = 0; i < prc1->code_size; i++)
    {
        printf ("  [%zu] " STACK_ELEM "\n", i, prc1->code[i]);
    }

    /*printf ("\n----- LABELS -----\n"); // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! починить
    for (int i = 0; i < LABEL_MAX; i++)
    {
        if (prc1->labels[i] != -1)
            printf ("  :%d -> %d\n", i, prc1->labels[i]);
    }*/
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

    //printf ("sum <"STACK_ELEM">\n", sum);
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
        printf ("SUB: not enough operands\n");
        return;
    }

    stack_elem_t Val1 = StackPop (stk, err);
    if (*err != STACK_OK)
    {
        printf ("SUB: not enough operands\n");
        return;
    }

    stack_elem_t difference = Val1 - Val2;

    //printf ("difference <"STACK_ELEM">\n", difference);
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

    //printf ("quotient <"STACK_ELEM">\n", quotient);
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

    //printf ("Multiplication <"STACK_ELEM">\n", multiplication);
    StackPush (stk, multiplication, err);

    *err = STACK_OK;
}

void Out (struct stack_t* stk, int* err)
{
    stack_elem_t out_elem = StackPop (stk, err); //!!!!!!!!!!!!!!!!!!!!!!!!!!название переменной 
    if (*err != STACK_OK)
    {
        printf ("OUT: stack is empty\n");
        return;
    }
    printf ("OP_OUT: ");
    //printf ("Out value: "STACK_ELEM"\n", out_elem;
    printf (STACK_ELEM"\n", out_elem);
    *err = STACK_OK;
}

int  Jump (struct processor_info* prc1, int* err)
{
    int target = (int)prc1->code[prc1->ip + 1];
    /*if ( prc1->labels_amount < LABEL_MAX)
    {
    prc1->labels[prc1->labels_amount++] = target; //пока просто записываем метки -- так метки не запишем
    }*/

    if (target < 0 || (size_t)target >= prc1->code_size)
    {
        printf ("JMP fail: bad address %d\n", target);
        return -1;
    }
    prc1->ip = (size_t)target;
    return 0;
}

int Jump_Above (struct stack_t* stk,struct processor_info* prc1, int* err)
{
    int target = (int)prc1->code[prc1->ip + 1];
    /*if ( prc1->labels_amount < LABEL_MAX)
    {
        prc1->labels[prc1->labels_amount++] = target; //пока просто записываем метки -- так метки не запишем
    }*/

    if (target < 0 || (size_t)target >= prc1->code_size)
    {
        printf ("JA fail: bad address %d\n", target);
        return -1;
    }

    stack_elem_t a = StackPop (stk, err);
    stack_elem_t b = StackPop (stk, err);
    if (*err != STACK_OK)
    {
        printf ("JA: stack underflow\n");
        return -1;
    }

    if (a > b) prc1->ip = (size_t)target;
    else       prc1->ip += 2;

    return 0;
}

int Call (struct stack_t* stk,struct processor_info* prc1, int* err)
{
    int target = (int)prc1->code[prc1->ip + 1];
    
    if (target < 0 || (size_t)target >= prc1->code_size)
    {
        printf ("JMP fail: bad address %d\n", target);
        return -1;
    }
    prc1->registers[4] = (int)prc1->code[prc1->ip];
    prc1->ip = (size_t)target;

    return 0;

}

int Ret (struct stack_t* stk,struct processor_info* prc1, int* err)
{

    return 1;
}


void Hlt (struct processor_info* prc1, int* err)
{

    printf ("\n\n===============================Processor finish work===============================\n");
    printf ("HLT at IP=%zu\n", prc1->ip);
    printf     ("===================================================================================\n");
}


int ReadFromFile (const char* read_file, char* index[], char** buffer, struct File_info* Command)
{
    assert (read_file != NULL);
    assert (index != NULL);

    FILE* Command_file = fopen (read_file, "rb");

    if (Command_file == NULL)
    {
        perror ("fopen");
        return 0;
    }

    int file_dis = fileno (Command_file);

    struct stat stat_file;

    fstat (file_dis, &stat_file);

    Command->size_file = stat_file.st_size;

    *buffer = calloc (Command->size_file + 1, sizeof (char));
    if (*buffer == NULL)
    {
        fclose (Command_file);
        return 0;
    }

    fread (*buffer, sizeof (char), Command->size_file, Command_file);
    (*buffer)[Command->size_file] = '\0';

    int max_rows = 0;

    index [0] = *buffer;

    for (int i = 0; i < Command->size_file; i ++)
    {
        if ( *(*buffer + i) == '\n')
        {
            *(*buffer + i) = '\0';
            max_rows ++;
            index [ max_rows] =  *buffer + i + 1;
        }
    }

    fclose (Command_file);

    return max_rows;
}
