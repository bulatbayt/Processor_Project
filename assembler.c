#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Opcode.h"

/* ==================================
 сделал бинарный файл, сделать другую переменную по тиму ip, на ip теперь тяжело 
*/
struct Command
{
    char* name;
    int   opcode;
};

struct LabelTable
{
    int labels[LABEL_MAX]; 
};

//============================================================================
//===========================Хэширование====================================== //неправильное хэширование
//============================================================================
struct Command commands[] =
{
    { "HLT",   OP_HLT },
    { "PUSH",  OP_PUSH },
    { "ADD",   OP_ADD },
    { "SUB",   OP_SUB },
    { "DIV",   OP_DIV },
    { "OUT",   OP_OUT },
    { "POP",   OP_POP },
    { "JMP",   OP_JMP },
    { "MUL",   OP_MUL },
    { "JA",    OP_JA },
    { "PUSHr", OP_PUSHr },
    { "CALL",  OP_CALL },
    { "RET",   OP_RET },
    { "PUSHm", OP_PUSHm },
    { "POPm",  OP_POPm },
    { "PUSHmr", OP_PUSHmr },
    { "POPmr",  OP_POPmr },
};

typedef int (*CommandFunc)(FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[]);


//============================================================================
//===========================Ассемблирование==================================
//============================================================================
int  ReadCommand       (char* file_read, char* file_write);
 
int  FirstAssembling   (char* file_read, char* file_write, struct LabelTable* labels, char** command_buffer);
int  SecondAssembling  (char* file_read, char* file_write, struct LabelTable* labels, char** command_buffer);
int  Translate_Command (const char* line, FILE* command_write, struct LabelTable* labels, int pass, size_t* ip);

//============================================================================
//===========================Вспомогательные функции==========================
//============================================================================
int  Find_Command   (char* command_name);
int  ParseRegister  (const char* s);
void LabelInit      (struct LabelTable* t);
void LabelAdd       (struct LabelTable* t, int label_number, size_t ip);
int  LabelFind      (struct LabelTable* t, int label_number);
int  marker         (const char* line, struct LabelTable* labels, size_t* ip);

//============================================================================
//===========================Командные функции================================
//============================================================================
int  Hlt       (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[]);
int  Push      (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[]);
int  Add       (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[]);
int  Sub       (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[]);
int  Div       (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[]);
int  Out       (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[]);
int  Pop       (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[]);
int  Jmp       (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[]);
int  Mul       (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[]);
int  Ja        (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[]);
int  Pushr     (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[]);
int  Call      (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[]);
int  Ret       (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[]);
int  Pushm     (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[]);
int  Popm      (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[]);
int  Pushmr    (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[]);
int  Popmr     (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[]);

CommandFunc command_table[] = 
{
    [OP_HLT]    = Hlt,
    [OP_PUSH]   = Push,
    [OP_ADD]    = Add,
    [OP_SUB]    = Sub,
    [OP_DIV]    = Div,
    [OP_OUT]    = Out,
    [OP_POP]    = Pop,
    [OP_JMP]    = Jmp,
    [OP_MUL]    = Mul,
    [OP_JA]     = Ja,
    [OP_PUSHr]  = Pushr,
    [OP_CALL]   = Call,
    [OP_RET]    = Ret,
    [OP_PUSHm]  = Pushm,
    [OP_POPm]   = Popm,
    [OP_PUSHmr] = Pushmr,
    [OP_POPmr]  = Popmr,
};


int main (int argc, char* argv[])
{
    ReadCommand ("test.asm", "Processor_write.bin");
    return 0;
}

//============================================================================
//===========================Ассемблирование==================================
//============================================================================

int ReadCommand (char* file_read, char* file_write)
{
    struct LabelTable labels = {};
    LabelInit (&labels);
    char* command_buffer = NULL;


    if ( FirstAssembling (file_read,file_write, &labels, &command_buffer) != 0) 
    {
        printf ("Review the errors\n");
        return 1;
    }
    
    if ( SecondAssembling (file_read,file_write, &labels, &command_buffer) != 0) 
    {
        printf ("Review the errors\n");
        return 1;
    }

    return 0;
}

int FirstAssembling (char* file_read, char* file_write, struct LabelTable* labels, char** command_buffer)
{
    FILE* pass1 = fopen (file_read, "r");
    if (pass1 == NULL)
    {
        printf ("Error open\n");
        return 1;
    }

    *command_buffer = calloc (SIZE_BUF, sizeof (char));
    if (*command_buffer == NULL)
    {
        fclose (pass1);
        return 1;
    }

    FILE* command_write = fopen (file_write, "wb"); // Бинарный режим
    if (command_write == NULL)
    {
        free (*command_buffer);
        fclose (pass1);
        return 1;
    }

    size_t ip = 0;

    while (fgets (*command_buffer, SIZE_BUF - 1, pass1) != NULL) 
    {
        int result = Translate_Command (*command_buffer, command_write,
                                     labels, 1, &ip);

        if (result == -1)
        {
            printf ("Error with Assembler (pass 1)\n");
            free   (*command_buffer);
            fclose (command_write);
            fclose (pass1);
            return 1;
        }
    }

    fclose (command_write);
    fclose (pass1);
    return 0;
}

int SecondAssembling (char* file_read, char* file_write, struct LabelTable* labels, char** command_buffer)
{
    FILE* command_read = fopen (file_read, "r");
    if (command_read == NULL)
    {
        free (*command_buffer);
        return 1;
    }

    FILE* command_write = fopen (file_write, "wb"); 
    if (command_write == NULL)
    {
        free (*command_buffer);
        fclose (command_read);
        return 1;
    }

    size_t ip = 0;

    while (fgets (*command_buffer, SIZE_BUF - 1, command_read) != NULL)
    {
        int result = Translate_Command (*command_buffer, command_write,
                                         labels, 2, &ip);

        if (result == -1)
        {
            printf ("Error with Assembler (pass 2)\n");
            break;
        }

        if (result == -2)
        {
            printf ("The programm completed successfully.\n");
        }
    }

    free (*command_buffer);

    fclose (command_write);
    fclose (command_read);
    return 0;
}

int Translate_Command (const char* line, FILE* command_write,
                    struct LabelTable* labels, int pass, size_t* ip) 
{
    char command_name[SIZE_BUF] = {};
    char argument[SIZE_BUF] = {};

    if ( marker (line, labels, ip) == 1)
    {
        return 1;
    }    

    sscanf (line, "%31s %31s", command_name, argument);

    int op = Find_Command (command_name); 

    if (op == -1)
    {
        printf ("Unknown command\n");
        return -1;
    }


    int result = command_table[op](command_write, ip, labels, argument);
    if (result == -1) 
    { 
        printf("Error\n"); 
        return -1; 
    }
    if (result == -2) 
    { 
        return -2; 
    }
    
    return 0;
}

// crc32
int Find_Command (char* command_name)
{
    for ( size_t i = 0; commands[i].name != NULL; i++)
    {
        if (strcmp (commands[i].name, command_name) == 0)
        {
            return commands[i].opcode;
        }
    }
    return -1;
}


//============================================================================
//===========================Вспомогательные функции==========================
//============================================================================
int ParseRegister (const char* s)
{
    if (strcmp (s, "AX") == 0) return 0;
    if (strcmp (s, "BX") == 0) return 1;
    if (strcmp (s, "CX") == 0) return 2;
    if (strcmp (s, "DX") == 0) return 3;
    return -1;
}

void LabelInit (struct LabelTable* t)
{
    for (int i = 0; i < LABEL_MAX; i++)
        t->labels[i] = -1;
}

void LabelAdd (struct LabelTable* t, int label_number, size_t ip)
{
    if (label_number < 0 || label_number >= LABEL_MAX) return;
    t->labels[label_number] = (int)ip;
}

int LabelFind (struct LabelTable* t, int label_number)
{
    if (label_number < 0 || label_number >= LABEL_MAX) 
        return -1;
    return t->labels[label_number];
}

int marker (const char* line, struct LabelTable* labels, size_t* ip)
{
    if (line[0] == ':') 
    {
        int label_number = atoi (line + 1);

        if (label_number >= 0 && label_number < LABEL_MAX)
        {
            labels->labels[label_number] = (int)(*ip);
        }
        return 1;
    }
    return 0;
}

//============================================================================
//===========================Комадные функции=================================
//============================================================================
int  Hlt   (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[])
{
    unsigned char opcode = OP_HLT;
    fwrite (&opcode, sizeof(unsigned char), 1, command_write);
    *ip += 1;
    return -2;
}

int  Push  (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[])
{
    unsigned char opcode = OP_PUSH;
    double value = atof (argument);
    
    fwrite (&opcode, sizeof(unsigned char), 1, command_write);
    fwrite (&value,  sizeof(double),        1, command_write);
    
    *ip += 1 + sizeof(double);
    return 0;
}

int  Add   (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[])
{
    unsigned char opcode = OP_ADD;
    fwrite (&opcode, sizeof(unsigned char), 1, command_write);
    *ip += 1;
    return 0;
}

int  Sub   (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[])
{
    unsigned char opcode = OP_SUB;
    fwrite (&opcode, sizeof(unsigned char), 1, command_write);
    *ip += 1;
    return 0;
}

int  Div   (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[])
{
    unsigned char opcode = OP_DIV;
    fwrite (&opcode, sizeof(unsigned char), 1, command_write);
    *ip += 1;
    return 0;
}

int  Out   (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[])
{
    unsigned char opcode = OP_OUT;
    fwrite (&opcode, sizeof(unsigned char), 1, command_write);
    *ip += 1;
    return 0;
}

int Pop    (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[])
{
    int reg = ParseRegister (argument);
    if (reg == -1)
    {
        printf ("Unknown register: %s\n", argument);
        return -1;
    }

    unsigned char opcode = OP_POP;
    fwrite (&opcode, sizeof(unsigned char), 1, command_write);
    fwrite (&reg,    sizeof(int),           1, command_write);
    
    *ip += 1 + sizeof(int);
    return 0;
}

int  Jmp   (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[]) 
{
    int label_number;

    if (argument[0] == ':')
        label_number = atoi (argument + 1);
    else
        label_number = atoi (argument);

    int label = LabelFind (labels, label_number);

    unsigned char opcode = OP_JMP;
    fwrite (&opcode, sizeof(unsigned char), 1, command_write);
    fwrite (&label,  sizeof(int),           1, command_write);
    
    *ip += 1 + sizeof(int);
    return 0;
}

int  Mul   (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[])
{
    unsigned char opcode = OP_MUL;
    fwrite (&opcode, sizeof(unsigned char), 1, command_write);
    *ip += 1;
    return 0;
}

int  Ja    (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[])
{
    int label_number;

    if (argument[0] == ':')
        label_number = atoi (argument + 1);
    else
        label_number = atoi (argument);

    int label = LabelFind (labels, label_number);

    unsigned char opcode = OP_JA;
    fwrite (&opcode, sizeof(unsigned char), 1, command_write);
    fwrite (&label,  sizeof(int),           1, command_write);
    
    *ip += 1 + sizeof(int);
    return 0;
}

int Pushr  (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[])
{
    int reg = ParseRegister (argument);
    if (reg == -1)
    {
        printf ("Unknown register: %s\n", argument);
        return -1;
    }

    unsigned char opcode = OP_PUSHr;
    fwrite (&opcode, sizeof(unsigned char), 1, command_write);
    fwrite (&reg,    sizeof(int),           1, command_write);
    
    *ip += 1 + sizeof(int);
    return 0;
}

int  Call  (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[])
{
    int label_number;

    if (argument[0] == ':')
        label_number = atoi (argument + 1);
    else
        label_number = atoi (argument);

    int label = LabelFind (labels, label_number);

    unsigned char opcode = OP_CALL;
    fwrite (&opcode, sizeof(unsigned char), 1, command_write);
    fwrite (&label,  sizeof(int),           1, command_write);
    
    *ip += 1 + sizeof(int);
    return 0;
}

int  Ret   (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[])
{
    unsigned char opcode = OP_RET;
    fwrite (&opcode, sizeof(unsigned char), 1, command_write);
    *ip += 1;
    return 0;
}

int Pushm  (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[])
{
    int value = 0;

    if (argument [0] == '[')
    {
        value = atoi (argument + 1); 
    }
    else
    {
        printf ("Error with syntax\n");
        return -1;
    }

    unsigned char opcode = OP_PUSHm;
    fwrite (&opcode, sizeof(unsigned char), 1, command_write);
    fwrite (&value,  sizeof(int),           1, command_write);
    
    *ip += 1 + sizeof(int);
    return 0;
}

int Popm   (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[])
{
    int value = 0;

    if (argument [0] == '[') 
    {
        value = atoi (argument + 1); 
    }
    else
    {
        printf ("Error with syntax\n");
        return -1;
    }

    unsigned char opcode = OP_POPm;
    fwrite (&opcode, sizeof(unsigned char), 1, command_write);
    fwrite (&value,  sizeof(int),           1, command_write);
    
    *ip += 1 + sizeof(int);
    return 0;
}

int Pushmr (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[])
{
    char registers[5] = { };

    if (argument [0] == '[') 
    {
        size_t i = 1;
        size_t j = 0;
        while ( i < 5 && argument[i] != ']' && argument[i] != '\0')
        {
            registers[j] = argument[i];
            i++;
            j++;
        }
        registers[j] = '\0';
    }
    else
    {
        printf ("Error with syntax\n");
        return -1;
    }

    int reg = ParseRegister (registers);
    if (reg == -1)
    {
        printf ("Unknown register: %s\n", registers);
        return -1;
    }

    unsigned char opcode = OP_PUSHmr;
    fwrite (&opcode, sizeof(unsigned char), 1, command_write);
    fwrite (&reg,    sizeof(int),           1, command_write);
    
    *ip += 1 + sizeof(int);
    return 0;
}

int Popmr  (FILE* command_write, size_t* ip, struct LabelTable* labels, char argument[])
{
    char registers[5] = { };

    if (argument [0] == '[') 
    {
        size_t i = 1;
        size_t j = 0;
        while ( i < 5 && argument[i] != ']' && argument[i] != '\0')
        {
            registers[j] = argument[i];
            i++;
            j++;
        }
        registers[j] = '\0';
    }
    else
    {
        printf ("Error with syntax\n");
        return -1;
    }

    int reg = ParseRegister (registers);
    if (reg == -1)
    {
        printf ("Unknown register: %s\n", registers);
        return -1;
    }

    unsigned char opcode = OP_POPmr;
    fwrite (&opcode, sizeof(unsigned char), 1, command_write);
    fwrite (&reg,    sizeof(int),           1, command_write);
    
    *ip += 1 + sizeof(int);
    return 0;
}
