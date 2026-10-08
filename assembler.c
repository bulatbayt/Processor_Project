#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ReadCommand (char* file_name);
int ExecuteCommand (const char* line, FILE* command_write);

int main (int argc, char* argv[]) // argv для файла считывания / записывания
{

    ReadCommand ("code.asm");

    return 0;
}


int ReadCommand (char* file_name)
{
    FILE* command_read = fopen (file_name, "r");

    if (command_read == NULL)
    {
        printf ("Error open\n");
        return 1;
    }
    
    char* comand_buffer = calloc (32, sizeof(char));
    
    FILE* command_write = fopen ("Processor_write.asm", "w");

    while (fgets (comand_buffer, 32-1, command_read) != NULL)
    {
        int result = ExecuteCommand(comand_buffer, command_write);

        if (result == -1)
        {
            printf ("Error with Assembler");
            break;
        }
    }

    fclose (command_write);
    fclose (command_read);
}

int ExecuteCommand (const char* line, FILE* command_write)
{
    char command_name[32] = {};
    double value = 0;


    if (sscanf (line, "%31s %lf", command_name, &value) == 2)
    {
        if (strcmp (command_name, "PUSH") == 0)
        {
            fprintf (command_write, "%d %lg\n", 1, value);
            return 1;
        }
    }

    
    if (sscanf (line, "%31s", command_name) == 1)
    {
        if (strcmp (command_name, "ADD") == 0) 
        { 
            fprintf (command_write, "%d\n", 2);
            return 1;
        }
        if (strcmp (command_name, "SUB") == 0) 
        { 
            fprintf (command_write, "%d\n", 3);
            return 1;
        }
        if (strcmp (command_name, "DIV") == 0) 
        { 
            fprintf (command_write, "%d\n", 4);
            return 1;
        }
        if (strcmp (command_name, "OUT") == 0) 
        { 
            fprintf (command_write, "%d\n", 5);
            return 1;
        }
        if (strcmp (command_name, "HLT") == 0) 
        { 
            fprintf (command_write, "%d\n", 0);
            return 0; 
        }  

        printf ("Unknown command: %s\n", command_name);
        return -1;
    }

    return 1;   
}