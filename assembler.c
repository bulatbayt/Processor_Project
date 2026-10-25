#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Opcode.h"


struct Command
{
    unsigned int hash_code;
    char*        name;
    int          opcode;
};

struct LabelTable
{
    int labels[LABEL_MAX]; 
};

struct Hash_Table
{
    unsigned int* table;
};

//============================================================================
//===========================Хэширование====================================== //неправильное хэширование
//============================================================================
struct Command commands[] =  // глобальная переменная, скорее всего это плохо, можно сделать ее не глобальной, нужно будет передавать в функции
{
    {0, "HLT",   OP_HLT    },
    {0, "PUSH",  OP_PUSH   },
    {0, "ADD",   OP_ADD    },
    {0, "SUB",   OP_SUB    },
    {0, "DIV",   OP_DIV    },
    {0, "OUT",   OP_OUT    },
    {0, "POP",   OP_POP    },
    {0, "JMP",   OP_JMP    },
    {0, "MUL",   OP_MUL    },
    {0, "JA",    OP_JA     },
    {0, "PUSHr", OP_PUSHr  },
    {0, "CALL",  OP_CALL   },
    {0, "RET",   OP_RET    },
    {0, "PUSHm", OP_PUSHm  },
    {0, "POPm",  OP_POPm   },
    {0, "PUSHmr", OP_PUSHmr},
    {0, "POPmr",  OP_POPmr },
};


int          Hash_Table_Init (struct Command Commands[]);
unsigned int Crc32           (const char* s);
size_t       binary_search   (unsigned int value, struct Command commands[], size_t size_data);

//============================================================================
//============================================================================

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
int  Find_Command          (char* command_name); // уже не нужна
int  ParseRegister         (const char* s);
void LabelInit             (struct LabelTable* t);
void LabelAdd              (struct LabelTable* t, int label_number, size_t ip);
int  LabelFind             (struct LabelTable* t, int label_number);
int  marker                (const char* line, struct LabelTable* labels, size_t* ip);
int  Compare_struct_Up     (const void* value_1, const void* value_2);
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
/*
создадим функцию хэширования, сделаем табличку (где?)
будем сравнивать числа ( с таблички)

1) создать табличку в самом начале -> пробежаться по массиву, чтобы сделать хэш таблицу, Но нужно завести новую переменную, которая будет считать только команды (например cp)
2) 1 пункт фигня - делаем то же самое, нужно ручками сделать хэш таблицу для каждого оп кода, при чтении, сравнивать их как числа <_---- как переводить строку в число ?
3) после перевода текста в число (какой то алгоритм) пробегаемся по хэш таблице бинарным поиском

текующая проблема - массив не отсортирован, если отсортировать потеряем порядок команд, который давал инфу, что это за операция
сделать массив структур, который будет содержать опкод, как было до этого


Проблемы: 1)binary_search сломается если не найдет такой команды
2)

*/

int main (int argc, char* argv[])
{
    Hash_Table_Init (commands);
    
    qsort (commands, SIZE_OPCODE, sizeof(struct Command), Compare_struct_Up); 

    ReadCommand ("test.asm", "Processor_write.bin");
    return 0;
}

//============================================================================
//===========================Ассемблирование==================================
//============================================================================

int ReadCommand       (char* file_read, char* file_write)
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

int FirstAssembling   (char* file_read, char* file_write, struct LabelTable* labels, char** command_buffer)
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

int SecondAssembling  (char* file_read, char* file_write, struct LabelTable* labels, char** command_buffer)
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

int Translate_Command (const char* line, FILE* command_write, struct LabelTable* labels, int pass, size_t* ip) 
{
    char command_name[SIZE_BUF] = {};
    char argument[SIZE_BUF] = {};

    if ( marker (line, labels, ip) == 1)
    {
        return 1;
    }    

    sscanf (line, "%31s %31s", command_name, argument); // обработать возможные ошибки

    unsigned int op_hash = Crc32(command_name);

    //printf ("I'm here\n");

    int op = commands[binary_search (op_hash, commands, SIZE_OPCODE)].opcode; // в нашей таблице каждому hash свой порядковый номер

    //printf ("Find op = %d\n", op);

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

//============================================================================
//===========================Хэширование =====================================
//============================================================================

unsigned int Crc32           (const char* s)
{
    unsigned int crc = 0xFFFFFFFF;
    
    while (*s != '\0')
    {
        crc ^= (unsigned char)*s;
        for (int i = 0; i < 8; i++)
        {
            if (crc & 1)
            {
                crc = (crc >> 1) ^ 0xEDB88320;
            }
            else
            {
                crc = crc >> 1;
            }
        }
        s++;
    }
    return crc ^ 0xFFFFFFFF;
}

int          Hash_Table_Init (struct Command commands[])
{
    for (size_t i = 0; i < SIZE_OPCODE; i++)
    {
        commands[i].hash_code = Crc32 (commands[i].name);
    }

    return 0;
}

size_t       binary_search   (unsigned int value, struct Command commands[], size_t size_data)
{
    size_t i = (int)((size_data) / 2);

    size_t infimum = 0;
    size_t supremum = size_data;
    
    while (infimum < supremum)
    {
        if (commands[i].hash_code < value)
        {
            infimum = i;
            i =infimum + (int)((supremum - infimum)/2);
        }

        if (commands[i].hash_code > value)
        {
            supremum = i;
            i = infimum + (int)((supremum - infimum)/2);
        }

        if (commands[i].hash_code == value)
        {
            return i;
        }
    }
}

//============================================================================
//===========================Вспомогательные функции==========================
//============================================================================
int  ParseRegister     (const char* s)
{
    if (strcmp (s, "AX") == 0) return 0;
    if (strcmp (s, "BX") == 0) return 1;
    if (strcmp (s, "CX") == 0) return 2;
    if (strcmp (s, "DX") == 0) return 3;
    return -1;
}

void LabelInit         (struct LabelTable* t)
{
    for (int i = 0; i < LABEL_MAX; i++)
        t->labels[i] = -1;
}

void LabelAdd          (struct LabelTable* t, int label_number, size_t ip)
{
    if (label_number < 0 || label_number >= LABEL_MAX) return;
    t->labels[label_number] = (int)ip;
}

int  LabelFind         (struct LabelTable* t, int label_number)
{
    if (label_number < 0 || label_number >= LABEL_MAX) 
        return -1;
    return t->labels[label_number];
}

int  marker            (const char* line, struct LabelTable* labels, size_t* ip)
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

int  Compare_struct_Up (const void* struct_1, const void* struct_2)
{
    unsigned int a = ((struct Command*)struct_1)->hash_code;
    unsigned int b = ((struct Command*)struct_2)->hash_code;
    if (a < b)
    {
        return -1;
    }

    if (a > b)
    {
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
