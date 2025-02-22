#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "dshlib.h"
extern void print_dragon();


static int last_command_exit_code = 0; //return code of the last command (for the rc command)
/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */


// function to parse input lines into a cmd_buff 
static void parse(char *input, cmd_buff_t *cmd) {
    cmd->argc = 0;

    cmd->_cmd_buffer = strdup(input); //duplicate the input string to cmd->_cmd_buffer

    if (!cmd->_cmd_buffer)  //check if memory allocation was successful
    {
        fprintf(stderr, "Memory allocation error\n");
        exit(ERR_MEMORY);
    }
    
    char *chr = cmd->_cmd_buffer; //pointer to traverse the input string
    //iterate over the string
    while (*chr != '\0') {
        while (*chr && isspace((unsigned char)*chr))
            chr++; // skip over leading space
        if (*chr == '\0') //if end is reached, break
            break;
        
        if (*chr == '"') //if argument starts with a quote
        {
            chr++;  //skip over first quote
            char *start = chr; //mark the start of the quoted argument
            while (*chr && *chr != '"') //traverse the quoted argument
                chr++;
            if (*chr == '"') {
                *chr = '\0';  //terminate when end quote found
                chr++;        //skip end quote
            }
            cmd->argv[cmd->argc++] = start; //store the argument in argv

        } else // if argument not in quotes
        {
            char *start = chr;
            while (*chr && !isspace((unsigned char)*chr)) //traverse until whitespace or end of string
                chr++;
            if (*chr) //if whitespace found
            {
                *chr = '\0'; //null terminate
                chr++; //skip nul; terminating character
            }
            cmd->argv[cmd->argc++] = start; //store the argument in argv
        }
        if (cmd->argc >= CMD_ARGV_MAX - 1) //check if number of arguments is within limit
            break;
    }
    cmd->argv[cmd->argc] = NULL; //Null terminate argv
}

int exec_local_cmd_loop()
{
    char input[ARG_MAX];
    cmd_buff_t cmd;
    int rc = OK;

   // read input stream
    while (fgets(input, sizeof(input), stdin) != NULL) {
        // Skip trailing \n
        input[strcspn(input, "\n")] = '\0';
        
        if (strlen(input) == 0) {
            printf(CMD_WARN_NO_CMD); //Error for no commands
        }

        else if (strcmp(input, EXIT_CMD) == 0) {
            break; //exit on exit command
        }
        else if (strcmp(input, "dragon") == 0) {
            print_dragon(); //print ascii art on dragon command
        }
        else if (strcmp(input, "rc") == 0) {
            printf("%d\n", last_command_exit_code); //return last exit code for rc command
        }
        // cd command
        else if (strncmp(input, "cd", 2) == 0) {
            parse(input, &cmd);
            if (cmd.argc == 1) {
                //no argument
            } else if (cmd.argc == 2) {
                if (chdir(cmd.argv[1]) != 0) //chdir() the dsh process into the directory provided 
                    last_command_exit_code = errno; //change exit code to indicate error
            }
            free(cmd._cmd_buffer); //free buffer
        }
        // for external commands
        else {
            parse(input, &cmd); //parse input into a cmd_buff

            if (cmd.argc == 0) {
                printf(CMD_WARN_NO_CMD); //error for no commands
                free(cmd._cmd_buffer); //free buffer

            } else {
                pid_t p = fork(); //store process ID of fork
                if (p < 0) //child process not created
                {
                    perror("Unsuccessful fork");
                    free(cmd._cmd_buffer);
                } else if (p == 0) //child process created
                {
                    // execute the ext command
                    execvp(cmd.argv[0], cmd.argv);

                    int err = errno; //return error code from child process
                    if (err == ENOENT) //not found
                        fprintf(stderr, "Command not found in PATH\n");
                    else if (err == EACCES)//permission denied
                        fprintf(stderr, "Permission denied\n");
                    else
                        perror(CMD_ERR_EXECUTE); //execution failure
                    exit(err); //exit with error status

                } else //parent process
                {
                    int child_status; //to store status of child process
                    waitpid(p, &child_status, 0);
                    last_command_exit_code = WEXITSTATUS(child_status); //extract errno
                }
                free(cmd._cmd_buffer); //free buffer
            }
        }
        
        //print prompt after a command is processed
        printf("%s", SH_PROMPT);
        fflush(stdout);
    }
    
    //Print prompt at the end of processing until EOF
    printf("%s", SH_PROMPT);
    fflush(stdout);
    
    return rc;
}


