#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE_BUF    256
#define LABEL_MAX   4

/* todo 
хэш инструкции ( сравнивать с цифрами ), делить на функции? ---- achieved
сделать ret and call
навести чистоту как в VM.c, делить функции
*/
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

struct Command
{
    char* name;;
    int   opcode;
};

struct LabelTable
{
    int labels[LABEL_MAX]; 
};

//============================================================================
//===========================Хэширование======================================
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
};

//============================================================================
//===========================Ассемблирование==================================
//============================================================================
int  ReadCommand       (char* file_read, char* file_write);
 
int  FirstAssembling   (char* file_read, char* file_write, struct LabelTable* labels, char** command_buffer);
int  SecondAssembling  (char* file_read, char* file_write, struct LabelTable* labels, char** command_buffer);

int  Translate_Command (const char* line, FILE* command_write,
                     struct LabelTable* labels, int pass, size_t* ip);

//============================================================================
//===========================Вспомогательные функции==========================
//============================================================================
int Find_Command    (char* command_name);
int  ParseRegister  (const char* s);
void LabelInit      (struct LabelTable* t);
void LabelAdd       (struct LabelTable* t, int label_number, size_t ip);
int  LabelFind      (struct LabelTable* t, int label_number);
int marker          (const char* line, struct LabelTable* labels, size_t* ip);

//============================================================================
//===========================Командные функции================================
//============================================================================
void Hlt       (FILE* command_write, size_t* ip);
void Push      (FILE* command_write, size_t* ip, char argument[]);
void Add       (FILE* command_write, size_t* ip);
void Sub       (FILE* command_write, size_t* ip);
void Div       (FILE* command_write, size_t* ip);
void Out       (FILE* command_write, size_t* ip);
int  Pop       (FILE* command_write, size_t* ip, char argument[]);
void Jmp       (FILE* command_write,  struct LabelTable* labels, size_t* ip, char argument[]);
void Mul       (FILE* command_write, size_t* ip);
void Ja        (FILE* command_write,  struct LabelTable* labels, size_t* ip, char argument[]);
int Pushr      (FILE* command_write, size_t* ip, char argument[]);
void Call      (FILE* command_write,  struct LabelTable* labels, size_t* ip, char argument[]);


int main (int argc, char* argv[]) // юзать аргс аргв
{
    
    ReadCommand ("code.asm", "Processor_write.asm");
    return 0;
}

//============================================================================
//===========================Ассемблирование==================================
//============================================================================

int ReadCommand (char* file_read, char* file_write)
{
    struct LabelTable labels = {};
    LabelInit (&labels);
    char** command_buffer;


    if ( FirstAssembling (file_read,file_write, &labels, command_buffer) != 0) 
    {
        printf ("Review the errors\n");
        return 1;
    }
    
    if ( SecondAssembling (file_read,file_write, &labels, command_buffer) != 0) 
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

    FILE* command_write = fopen (file_write, "w");
    if (command_write == NULL)
    {
        free (*command_buffer);
        fclose (pass1);
        return 1;
    }

    size_t ip = 0;

    while (fgets (*command_buffer, SIZE_BUF - 1, pass1) != NULL) // здесь сделать хэши   -----achieved
    {
        //printf ("IP %zu\n", ip);
        int result = Translate_Command (*command_buffer, command_write,
                                     labels, 1, &ip);

        if (result == -1)
        {
            printf ("Error with Assembler (pass 1)\n");
            free (*command_buffer);
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

    FILE* command_write = fopen (file_write, "w");
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


    switch (op) // под каждый кейс функцию !!!!!!!!!!!!!!!!!!!
    {
        case 1:
        {
            Push (command_write, ip, argument);
            return 1;
            break;
        }

        case 2:
        {
            Add (command_write, ip);
            return 1;
        }

        case 3:
        {
            Sub (command_write, ip);
            return 1;
        }

        case 4:
        {
            Div (command_write, ip);
            return 1;
        }

        case 5:
        {
            Out (command_write, ip);
            return 1;
        }

        case 6:
        {
            Pop (command_write, ip, argument);
            return 1;
        }

        case 7:
        {
            Jmp (command_write, labels, ip, argument);
            return 1;
        }

        case 8:
        {
            Mul (command_write, ip);
            return 1;
        }

        case 9:
        {
            Ja (command_write, labels, ip, argument);
            return 1;   
        }

        case 10:
        {
            Pushr (command_write, ip, argument);
            return 1;
        }

        case 11:
        {
            Call (command_write, labels, ip, argument);
            return 1;
        }

        case 12: ////здесь ret !!!!!!!!!!!!!!!!!!!!!!!!!!!!
        {
            break;
        }

        case 0:
        {
            Hlt (command_write, ip);
            return -2;
        }

        default:
        {
            printf ("Unknown command\n");
            return -1;
        }
    }

    return 0;
}

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
    if (line[0] == ':') // функция метка
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
void Hlt (FILE* command_write, size_t* ip)
{
    fprintf (command_write, "%d\n", OP_HLT);
    *ip += 1;
}

void Push (FILE* command_write, size_t* ip, char argument[])
{
    double value = atof (argument);
    fprintf (command_write, "%d %lg\n", OP_PUSH, value);
    *ip += 2;
    
}

void Add (FILE* command_write, size_t* ip)
{
    fprintf (command_write, "%d\n", OP_ADD);
    *ip += 1;
}

void Sub (FILE* command_write, size_t* ip)
{
    fprintf (command_write, "%d\n", OP_SUB);
    *ip += 1;
}

void Div (FILE* command_write, size_t* ip)
{
    fprintf (command_write, "%d\n", OP_DIV);
    *ip += 1;
}

void Out (FILE* command_write, size_t* ip)
{
    fprintf (command_write, "%d\n", OP_OUT);
    *ip += 1;
}

int Pop (FILE* command_write, size_t* ip, char argument[])
{
    int reg = ParseRegister (argument);
    if (reg == -1)
    {
        printf ("Unknown register: %s\n", argument);
        return -1;
    }

    fprintf (command_write, "%d %d\n", OP_POP, reg);
    *ip += 2;
}

void Jmp (FILE* command_write,  struct LabelTable* labels, size_t* ip, char argument[])
{
    int label_number;

    if (argument[0] == ':')
        label_number = atoi (argument + 1);
    else
        label_number = atoi (argument);

    int label = LabelFind (labels, label_number);

    fprintf (command_write, "%d %d\n", OP_JMP, label);
    *ip += 2;
}

void Mul (FILE* command_write, size_t* ip)
{
    fprintf (command_write, "%d\n", OP_MUL);
    *ip += 1;
}


void Ja (FILE* command_write,  struct LabelTable* labels, size_t* ip, char argument[])
{
    int label_number;

    if (argument[0] == ':')
        label_number = atoi (argument + 1);
    else
        label_number = atoi (argument);

    int label = LabelFind (labels, label_number);

    fprintf (command_write, "%d %d\n", OP_JA, label);
    *ip += 2;
}

int Pushr (FILE* command_write, size_t* ip, char argument[])
{
    int reg = ParseRegister (argument);
    if (reg == -1)
    {
        printf ("Unknown register: %s\n", argument);
        return -1;
    }

    fprintf (command_write, "%d %d\n", OP_PUSHr, reg);
    *ip += 2;
}

void Call (FILE* command_write,  struct LabelTable* labels, size_t* ip, char argument[])
{
    int label_number;

    if (argument[0] == ':')
        label_number = atoi (argument + 1);
    else
        label_number = atoi (argument);

    int label = LabelFind (labels, label_number);

    fprintf (command_write, "%d %d\n", OP_CALL, label);
    *ip += 2;
}

/*void Ret ()
{

}*/
