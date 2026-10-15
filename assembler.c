#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE_BUF    256
#define LABEL_MAX   4

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

struct LabelTable
{
    int labels[LABEL_MAX]; 
};

int  ReadCommand    (char* file_read, char* file_write);
int  Translate_Command (const char* line, FILE* command_write,
                     struct LabelTable* labels, int pass, size_t* ip);

int  ParseRegister  (const char* s);
void LabelInit      (struct LabelTable* t);
void LabelAdd       (struct LabelTable* t, int label_number, size_t ip);
int  LabelFind      (struct LabelTable* t, int label_number);



int main (int argc, char* argv[]) // юзать аргс аргв
{

    ReadCommand ("code.asm", "Processor_write.asm");

    return 0;
}



int ReadCommand (char* file_read, char* file_write)
{
    struct LabelTable labels = {};
    LabelInit (&labels);

    // 
    // первый пошел
    // 
    FILE* pass1 = fopen (file_read, "r");
    if (pass1 == NULL)
    {
        printf ("Error open\n");
        return 1;
    }

    char* comand_buffer = calloc (SIZE_BUF, sizeof (char));
    if (comand_buffer == NULL)
    {
        fclose (pass1);
        return 1;
    }

    FILE* command_write = fopen (file_write, "w");
    if (command_write == NULL)
    {
        free (comand_buffer);
        fclose (pass1);
        return 1;
    }

    size_t ip = 0;

    while (fgets (comand_buffer, SIZE_BUF - 1, pass1) != NULL) // мб fread om прочитать 
    {
        int result = Translate_Command (comand_buffer, command_write,
                                     &labels, 1, &ip);

        if (result == -1)
        {
            printf ("Error with Assembler (pass 1)\n");
            free (comand_buffer);
            fclose (command_write);
            fclose (pass1);
            return 1;
        }
    }

    fclose (command_write);
    fclose (pass1);

    // 
    // второй пошел
    // 
    FILE* command_read = fopen (file_read, "r");
    if (command_read == NULL)
    {
        free (comand_buffer);
        return 1;
    }

    command_write = fopen (file_write, "w");
    if (command_write == NULL)
    {
        free (comand_buffer);
        fclose (command_read);
        return 1;
    }

    ip = 0;

    while (fgets (comand_buffer, SIZE_BUF - 1, command_read) != NULL)
    {
        int result = Translate_Command (comand_buffer, command_write,
                                     &labels, 2, &ip);

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

    free (comand_buffer);

    fclose (command_write);
    fclose (command_read);

    return 0;
}




int Translate_Command (const char* line, FILE* command_write,
                    struct LabelTable* labels, int pass, size_t* ip)
{
    char command_name[SIZE_BUF] = {};
    char argument[SIZE_BUF] = {};

    if (line[0] == ':')
    {
        int label_number = atoi (line + 1);

        if (label_number >= 0 && label_number < LABEL_MAX)
            labels->labels[label_number] = (int)(*ip);

        return 1;
    }

    
    if (sscanf (line, "%31s %31s", command_name, argument) == 2)
    {
        
        if (strcmp (command_name, "PUSH") == 0)
        {
            double value = atof (argument);
            fprintf (command_write, "%d %lg\n", OP_PUSH, value);
            *ip += 2;
            return 1;
        }

        if (strcmp (command_name, "PUSHr") == 0)
        {
            int reg = ParseRegister (argument);
            if (reg == -1)
            {
                printf ("Unknown register: %s\n", argument);
                return -1;
            }

            fprintf (command_write, "%d %d\n", OP_PUSHr, reg);
            *ip += 2;
            return 1;
        }

        
        if (strcmp (command_name, "POP") == 0)
        {
            int reg = ParseRegister (argument);
            if (reg == -1)
            {
                printf ("Unknown register: %s\n", argument);
                return -1;
            }

            fprintf (command_write, "%d %d\n", OP_POP, reg);
            *ip += 2;
            return 1;
        }

        

        if (strcmp (command_name, "JMP") == 0)
        {
            int label_number;

            if (argument[0] == ':')
                label_number = atoi (argument + 1); // если без : это значит прыгать не на метку, а на адрес
            else
                label_number = atoi (argument);

            int label = LabelFind (labels, label_number);


            fprintf (command_write, "%d %d\n", OP_JMP, label);
            *ip += 2;
            return 1;
        }

        
        if (strcmp (command_name, "JA") == 0)
        {
            int label_number;

            if (argument[0] == ':')
                label_number = atoi (argument + 1);
            else
                label_number = atoi (argument);

            int label = LabelFind (labels, label_number);

            fprintf (command_write, "%d %d\n", OP_JA, label);
            *ip += 2;
            return 1;
        }
    }

    
    if (sscanf (line, "%31s", command_name) == 1)
    {
        int code = -1;

        if (strcmp (command_name, "ADD") == 0) code = OP_ADD;
        if (strcmp (command_name, "SUB") == 0) code = OP_SUB;
        if (strcmp (command_name, "DIV") == 0) code = OP_DIV;
        if (strcmp (command_name, "OUT") == 0) code = OP_OUT;
        if (strcmp (command_name, "MUL") == 0) code = OP_MUL;

        if (code >= 0)
        {
            fprintf (command_write, "%d\n", code);
            *ip += 1;
            return 1;
        }

        if (strcmp (command_name, "HLT") == 0)
        {
            fprintf (command_write, "%d\n", OP_HLT);
            *ip += 1;
            return -2;
        }

        printf ("Unknown command: %s\n", command_name);
        return -1;
    }

    return 0;
}



// ============================================================
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
