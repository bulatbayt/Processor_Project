#include "stack_v3.h"
#include <sys/stat.h>
#include "Opcode.h"
/*todo
////////////////arc, argv для перенаправления ввода вывода
в константу или динамика --achieved
fread, fwrite -- бинарный тип , переход -- achieved
0) сделать функции полными, чтобы они делали сами все вещи из switcha --achieved
1) выставить одиннаковые сигнатуры функций --achieved
2) убрать свитч кэйс --achieved
бинарные операции
*/

#define REG_COUNT       6

struct processor_info
{
    stack_elem_t   registers[REG_COUNT]; // Ax Bx Cx CallX ---- как 
    size_t         ip;                    
    size_t         code_size;
    int            labels[LABEL_MAX];  
    size_t         labels_amount;
    int            err;
    unsigned char* code; // динамика
    size_t*        RAM;
};


typedef int (*command_func) (struct stack_t* stk, struct processor_info* prc1);

//=====================================================================================================================//
//=============================================Прототипы Вспомогательных функций ======================================//
//=====================================================================================================================//

void Ram_Init             (struct processor_info* prc, size_t size); 

void Label_Init           (struct processor_info* prc);

int  LoadFromBinary       (const char* read_file, struct processor_info* prc1);

int  realise_comand       (struct stack_t* stk, const char* filename, struct processor_info* prc1);

int  ProcessorVerify      (struct processor_info* prc1, struct stack_t* stk, int* err);

void CPU_Dump             (struct processor_info* prc1, FILE* write_CDump);

int  Start_Commands       (struct processor_info* prc1, FILE* write_CDump, int* err, struct stack_t* stk);

void Draw                 (struct processor_info prc1);

//=====================================================================================================================//
//=============================================Прототипы Команд Процессора=============================================//
//=====================================================================================================================//

int PushPrc    (struct stack_t* stk, struct processor_info* prc1);

int PopPrc     (struct stack_t* stk, struct processor_info* prc1);

int Add        (struct stack_t* stk, struct processor_info* prc1);

int Sub        (struct stack_t* stk, struct processor_info* prc1);

int Div        (struct stack_t* stk, struct processor_info* prc1);

int Mult       (struct stack_t* stk, struct processor_info* prc1);

int Out        (struct stack_t* stk, struct processor_info* prc1);

int Jump       (struct stack_t* stk, struct processor_info* prc1);

int Jump_Above (struct stack_t* stk, struct processor_info* prc1);

int Pushr      (struct stack_t* stk, struct processor_info* prc1);

int Call       (struct stack_t* stk, struct processor_info* prc1);

int Ret        (struct stack_t* stk, struct processor_info* prc1);

int Pushm      (struct stack_t* stk, struct processor_info* prc1);

int Popm       (struct stack_t* stk, struct processor_info* prc1);

int Pushmr     (struct stack_t* stk, struct processor_info* prc1);

int Popmr      (struct stack_t* stk, struct processor_info* prc1);

int Hlt        (struct stack_t* stk, struct processor_info* prc1);

//=====================================================================================================================//

command_func command_list[] =
{
    [OP_HLT]  = Hlt,
    [OP_PUSH] = PushPrc,
    [OP_ADD]  = Add,
    [OP_SUB]  = Sub,
    [OP_DIV]  = Div,
    [OP_OUT]  = Out,
    [OP_POP]  = PopPrc,
    [OP_JMP]  = Jump,
    [OP_MUL]    = Mult,
    [OP_JA]     = Jump_Above,
    [OP_PUSHr]  = Pushr,
    [OP_CALL]   = Call,
    [OP_RET]    = Ret,
    [OP_PUSHm]  = Pushm,
    [OP_POPm]   = Popm,
    [OP_PUSHmr]   = Pushmr,
    [OP_POPmr]   = Popmr,
};


int main (int argc, char* argv[]) 
{
    struct stack_t stk1 = {};
    struct processor_info prc1 = {};
    struct stack_t stk2 = {};


    Label_Init (&prc1);
    Ram_Init   (&prc1, 100);
    STACK_INIT (&stk1, 2, &prc1.err);
    STACK_INIT (&stk2, 2, &prc1.err); 


    printf (BLUE"\n========================Processing working========================\n"RESET_COLOR);
    
    realise_comand (&stk1, "Processor_write.bin", &prc1);

    printf ("========================The picture========================\n");
    Draw (prc1);
    StackDestroy (&stk1, &prc1.err);
    StackDestroy (&stk2, &prc1.err);

    if (prc1.code != NULL) 
    {
        free (prc1.code);
    }
    if (prc1.RAM != NULL)  
    {
        free (prc1.RAM);
    }
    return 0;
}

// ============================================================
// Вспомогательные функции
// ============================================================
void Ram_Init        (struct processor_info* prc, size_t size)
{
    prc->RAM = calloc (size, sizeof(stack_elem_t));
}

void Label_Init      (struct processor_info* prc)
{
    for (size_t i = 0; i < LABEL_MAX; i++) 
    {
        prc->labels[i] = -1;
    }
}

int  realise_comand  (struct stack_t* stk, const char* filename, struct processor_info* prc1)
{
    if ( LoadFromBinary (filename, prc1) != 0)
    {
        printf ("Error load binary\n");
        return -1;
    }

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
        printf ("Error with realise command, review the errors\n");
        fclose (write_CDump);
        return -1;
    }

    fclose (write_CDump);

    return 0;
}

int LoadFromBinary   (const char* read_file, struct processor_info* prc1)
{
    FILE* Command_file = fopen (read_file, "rb");

    if (Command_file == NULL)
    {
        perror ("fopen");
        return -1;
    }

    fseek (Command_file, 0, SEEK_END);
    size_t file_size = ftell (Command_file);
    fseek (Command_file, 0, SEEK_SET);

    if (file_size <= 0) 
    {
        printf ("Empty file\n");
        fclose (Command_file);
        return -1;
    }

    prc1->code = calloc (file_size, sizeof(char));
    if (prc1->code == NULL)
    {
        fclose (Command_file);
        return -1;
    }
    prc1->code_size = (size_t)file_size;

    fread (prc1->code, sizeof(char), prc1->code_size, Command_file);
    fclose (Command_file);

    return 0;
}

int  ProcessorVerify (struct processor_info* prc1, struct stack_t* stk1, int* err) 
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

void CPU_Dump        (struct processor_info* prc1, FILE* write_CDump) 
{
    #ifdef ONDEBUG
    fprintf (write_CDump, "\n----- CODE -----\n");
    for (size_t i = 0; i < prc1->code_size; i++)
    {
        fprintf (write_CDump, "  [%zu] %02X" , i, prc1->code[i]); // Выводим как HEX байт
        
        if (prc1->ip == i)
        {
            fprintf (write_CDump, "    [<---------");
        } 
        fprintf (write_CDump, "\n");
    }
    #endif
}

int  Start_Commands  (struct processor_info* prc1, FILE* write_CDump, int* err, struct stack_t* stk) 
{
    while (prc1->ip < prc1->code_size) 
    {
        unsigned char command_code = prc1->code[prc1->ip];
        CPU_Dump (prc1, write_CDump);
        
        prc1->ip += 1; // Сдвигаем на опкод
        // если функция возвращает -1 закроем все освободим и тд
        // hlt возвращает -2, тогда тоже закрываем файл
        //memcpy - это наверное долго в каждой функции 

        int result = command_list[command_code](stk, prc1);

        if (result == -1)
        {
            printf ("Error in func \"Starts_Commands\": return results = -1 \n");
            //fclose (write_CDump); уже закрываем после ретёрна start_commands
            return -1;
        }

        if (result == -2)
        {
            printf ("Finish processor work\n");
            return 0;
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

// ============================================================
// Команды Процессора
// ============================================================
int PushPrc    (struct stack_t* stk, struct processor_info* prc1)
{
    stack_elem_t value;
    memcpy (&value, &prc1->code[prc1->ip], sizeof (stack_elem_t));
    prc1->ip += sizeof (stack_elem_t);
    StackPush (stk, value, &prc1->err);
    return 0;
}

int PopPrc     (struct stack_t* stk, struct processor_info* prc1)
{
    int reg;
    memcpy (&reg, &prc1->code[prc1->ip], sizeof (int));
    prc1->ip += sizeof (int);
    
    stack_elem_t v = StackPop (stk, &prc1->err);
    if (prc1->err != STACK_OK)
    {
        printf ("POP: stack is empty\n");
        return -1;
    }
    prc1->registers[reg] = v;

    return 0;
}

int Pushr      (struct stack_t* stk, struct processor_info* prc1)
{
    int reg;
    memcpy (&reg, &prc1->code[prc1->ip], sizeof (int));
    
    prc1->ip += sizeof (int);

    StackPush (stk, prc1->registers[reg], &prc1->err);

    return 0;
}

int Add        (struct stack_t* stk, struct processor_info* prc1)
{
    assert (stk != NULL);
    assert (&prc1->err != NULL);

    stack_elem_t Val2 = StackPop (stk, &prc1->err);
    if (prc1->err != STACK_OK)
    {
        printf ("ADD: not enough operands\n");
        return -1;
    }

    stack_elem_t Val1 = StackPop (stk, &prc1->err);
    if (prc1->err != STACK_OK)
    {
        printf ("ADD: not enough operands\n");
        return -1;
    }

    stack_elem_t sum = Val1 + Val2;

    StackPush (stk, sum, &prc1->err);

    prc1->err = STACK_OK;
    return 0;
}

int Sub        (struct stack_t* stk, struct processor_info* prc1)
{
    assert (stk != NULL);
    assert (&prc1->err != NULL);

    stack_elem_t Val2 = StackPop (stk, &prc1->err);
    if (prc1->err != STACK_OK)
    {
        printf ("SUB: not enough operands\n");
        return -1;
    }

    stack_elem_t Val1 = StackPop (stk, &prc1->err);
    if (prc1->err != STACK_OK)
    {
        printf ("SUB: not enough operands\n");
        return -1;
    }

    stack_elem_t difference = Val1 - Val2;

    StackPush (stk, difference, &prc1->err);

    prc1->err = STACK_OK;
    return 0;
}

int Div        (struct stack_t* stk, struct processor_info* prc1)
{
    assert (stk != NULL);
    assert (&prc1->err != NULL);

    stack_elem_t Val2 = StackPop (stk, &prc1->err);
    if (prc1->err != STACK_OK)
    {
        printf ("DIV: not enough operands\n");
        return -1;
    }

    stack_elem_t Val1 = StackPop (stk, &prc1->err);
    if (prc1->err != STACK_OK)
    {
        printf ("DIV: not enough operands\n");
        return -1;
    }

    if (Val2 == 0)
    {
        printf ("DIV: division by zero\n");
        prc1->err = STACK_OK;
        return -1;
    }

    stack_elem_t quotient = Val1 / Val2;

    StackPush (stk, quotient, &prc1->err);

    prc1->err = STACK_OK;
    return 0;
}

int Mult       (struct stack_t* stk, struct processor_info* prc1)
{
    assert (stk != NULL);
    assert (&prc1->err != NULL);

    stack_elem_t Val2 = StackPop (stk, &prc1->err);
    if (prc1->err != STACK_OK)
    {
        printf ("Multiplication: not enough operands\n");
        return -1;
    }

    stack_elem_t Val1 = StackPop (stk, &prc1->err);
    if (prc1->err != STACK_OK)
    {
        printf ("Multiplication: not enough operands\n");
        return -1;
    }

    stack_elem_t multiplication = Val1 * Val2;

    StackPush (stk, multiplication, &prc1->err);

    prc1->err = STACK_OK;
    return 0;
}

int Out        (struct stack_t* stk, struct processor_info* prc1)
{
    stack_elem_t out_elem = StackPop (stk, &prc1->err);  
    if (prc1->err != STACK_OK)
    {
        printf ("OUT: stack is empty\n");
        return -1;
    }
    printf ("OP_OUT: ");
    printf (STACK_ELEM"\n", out_elem);
    prc1->err = STACK_OK;
    return 0;
}

int  Jump      (struct stack_t* stk, struct processor_info* prc1)
{
    int aim;
    memcpy (&aim, &prc1->code[prc1->ip], sizeof (int));
    prc1->ip += sizeof (int);

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
    int aim;
    memcpy (&aim, &prc1->code[prc1->ip], sizeof (int));
    prc1->ip += sizeof (int);

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

    return 0;
}

int Call       (struct stack_t* stk, struct processor_info* prc1)
{
    int aim;
    memcpy (&aim, &prc1->code[prc1->ip], sizeof (int));
    prc1->ip += sizeof (int);
    
    if (aim < 0 || (size_t)aim >= prc1->code_size)
    {
        printf ("Call fail: bad address %d\n", aim);
        return -1;
    }

    stack_elem_t push_value = (stack_elem_t)(prc1->ip);  

    StackPush (stk, push_value, &prc1->err);

    prc1->ip = (size_t)aim;
    return 0;
}

int Ret        (struct stack_t* stk, struct processor_info* prc1)
{
    prc1->ip = (size_t)StackPop (stk, &prc1->err);
    return 0;
}

int Pushm      (struct stack_t* stk, struct processor_info* prc1)
{
    int index;
    memcpy (&index, &prc1->code[prc1->ip], sizeof (int));
    prc1->ip += sizeof (int);
    
    StackPush (stk, prc1->RAM[index], &prc1->err);

    return 0;
}

int Popm       (struct stack_t* stk, struct processor_info* prc1)
{
    int index;
    memcpy (&index, &prc1->code[prc1->ip], sizeof (int));
    prc1->ip += sizeof (int);

    stack_elem_t value = StackPop (stk, &prc1->err);
    prc1->RAM[index] = value;

    return 0;
}

int Pushmr     (struct stack_t* stk, struct processor_info* prc1)
{
    int reg;
    memcpy (&reg, &prc1->code[prc1->ip], sizeof (int));
    prc1->ip += sizeof (int);

    StackPush (stk, prc1->RAM[(size_t)prc1->registers[reg]], &prc1->err);
    return 0;
}

int Popmr      (struct stack_t* stk, struct processor_info* prc1)
{
    int reg;
    memcpy (&reg, &prc1->code[prc1->ip], sizeof (int));
    prc1->ip += sizeof (int);

    stack_elem_t value = StackPop (stk, &prc1->err);
    prc1->RAM[(size_t)prc1->registers[reg]] = value;

    return 0;
}

int Hlt        (struct stack_t* stk, struct processor_info* prc1)
{

    printf     (BLUE"\n\n===============================Processor finish work===============================\n"RESET_COLOR);
    printf     ("HLT at IP=%zu\n", prc1->ip);
    printf     (BLUE    "===================================================================================\n"RESET_COLOR);
    return 0;
}

void  Draw     (struct processor_info prc1)
{
    for (size_t i = 0; i < 100; i++)
    {
        if (i % 10 == 0)
            printf ("\n");

        if (prc1.RAM[i] != 0)
        {
            printf (".");
        }
        if (prc1.RAM[i] == 0)
        {
            printf (" ");
        }
    }   
}
