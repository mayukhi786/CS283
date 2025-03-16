#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "dshlib.h"

extern void print_dragon();

#include <fcntl.h> //provides contants for file open operations (for input/output redirection)


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

    //set redirection fields
    cmd->input_redirect = NULL;
    cmd->output_redirect = NULL;
    cmd->append_mode = 0;

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

        //input redirection
        if (*chr == '<') {
            chr++; //skip over <
            //skip whitespace after '<'
            while (*chr && isspace((unsigned char)*chr))
                chr++;

            //pointer for start of filename
            char *filename_start = chr;
            
            //end of filename
            while (*chr && !isspace((unsigned char)*chr))
                chr++;
            
            //null terminate filename
            if (*chr) {
                *chr = '\0';
                chr++;
            }
            
            //store input filename
            cmd->input_redirect = filename_start;
            continue;
        }
        
        //output redirection
        if (*chr == '>') {
            //check for append mode '>>'
            if (*(chr+1) == '>') {
                cmd->append_mode = 1; //set flag for append mode to 1
                chr += 2; //skip the >>
            } else {
                cmd->append_mode = 0;
                chr++; //skip the >
            }
            
            //skip whitespace after '>' or '>>'
            while (*chr && isspace((unsigned char)*chr))
                chr++;
            
            //pointer for start of filename
            char *filename_start = chr;
            
            //end of filename
            while (*chr && !isspace((unsigned char)*chr))
                chr++;
            
            //null terminate filename
            if (*chr) {
                *chr = '\0';
                chr++;
            }
            
            //store output filename
            cmd->output_redirect = filename_start;
            continue;
        }
        
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


 //Function to parse command lines with multiple commands separated by pipes
int build_cmd_list(char *cmd_line, command_list_t *clist) {
    clist->num = 0;
    
    //make a copy of the cmd_line to tokenize
    char *line_copy = strdup(cmd_line);
    if (!line_copy) {
        return ERR_MEMORY;
    }
    
    //split the line by pipes
    char *token = strtok(line_copy, PIPE_STRING);
    while (token != NULL && clist->num < CMD_MAX) {
        //trim leading spaces
        while (*token && isspace((unsigned char)*token)) {
            token++;
        }
        
        //parse the command token
        parse(token, &clist->commands[clist->num]);
        clist->num++;
        
        //get next token
        token = strtok(NULL, PIPE_STRING);
    }
    
    free(line_copy);
    
    if (token != NULL && clist->num >= CMD_MAX) {
        return ERR_TOO_MANY_COMMANDS; //error for too many commands
    }
    
    return OK;
}

//Free memory allocated for command list
int free_cmd_list(command_list_t *clist) {
    for (int i = 0; i < clist->num; i++) {
        free(clist->commands[i]._cmd_buffer);
    }
    return OK;
}

//execute pipeline of commands
int execute_pipeline(command_list_t *clist) {
    if (clist->num == 0) {
        return WARN_NO_CMDS;
    }
    
    if (clist->num == 1) {
        //if there's only one command, execute it
        pid_t pid = fork();
        if (pid < 0) {
            perror("Unsuccesful fork");
            return ERR_EXEC_CMD;
        } else if (pid == 0) {
            //child process

            //handle input redirection
            if (clist->commands[0].input_redirect) {
                //Open read only file
                int input_fd = open(clist->commands[0].input_redirect, O_RDONLY);
                if (input_fd == -1) {
                    perror("Error opening input file");
                    exit(errno);
                }
                if (dup2(input_fd, STDIN_FILENO) == -1) {
                    perror("Error redirecting stdin");
                    close(input_fd);
                    exit(errno);
                }
                close(input_fd);
            }

            // Handle output redirection
            if (clist->commands[0].output_redirect) {
                //Open write only file or create new file
                //If append mode (>>), append to file else truncate file
                int output_flags = O_WRONLY | O_CREAT | (clist->commands[0].append_mode ? O_APPEND : O_TRUNC);
                int output_fd = open(clist->commands[0].output_redirect, output_flags, 0644); //open with appropriate permissions

                if (output_fd == -1) {
                    perror("Error opening output file");
                    exit(errno);
                }
                if (dup2(output_fd, STDOUT_FILENO) == -1) {
                    perror("Error redirecting stdout");
                    close(output_fd);
                    exit(errno);
                }
                close(output_fd);
            }

            //execute command
            execvp(clist->commands[0].argv[0], clist->commands[0].argv);
            perror(CMD_ERR_EXECUTE);
            exit(errno);
            
        } else {
            //parent process now
            int status;
            waitpid(pid, &status, 0);
            last_command_exit_code = WEXITSTATUS(status);
            return OK;
        }
    }
    
    //array to store pipe file descriptors
    int pipes[CMD_MAX - 1][2];
    pid_t pids[CMD_MAX];
    
    //create all necessary pipes
    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return ERR_EXEC_CMD;
        }
    }
    
    //processes for each command
    for (int i = 0; i < clist->num; i++) {
        pids[i] = fork();
        
        if (pids[i] < 0) {
            perror("fork");
            //close pipes
            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            return ERR_EXEC_CMD;
        }
        
        if (pids[i] == 0) {
            //child process

            //Handle input redirection for first command
            if (i == 0 && clist->commands[i].input_redirect) {
                int input_fd = open(clist->commands[i].input_redirect, O_RDONLY);
                if (input_fd == -1) {
                    perror("Error opening input file");
                    exit(errno);
                }
                if (dup2(input_fd, STDIN_FILENO) == -1) {
                    perror("Error redirecting stdin");
                    close(input_fd);
                    exit(errno);
                }
                close(input_fd);
            }

            //Handle output redirection for last command
            if (i == clist->num - 1 && clist->commands[i].output_redirect) {
                int output_flags = O_WRONLY | O_CREAT | (clist->commands[i].append_mode ? O_APPEND : O_TRUNC);
                int output_fd = open(clist->commands[i].output_redirect, output_flags, 0644);
                if (output_fd == -1) {
                    perror("Error opening output file");
                    exit(errno);
                }
                if (dup2(output_fd, STDOUT_FILENO) == -1) {
                    perror("Error redirecting stdout");
                    close(output_fd);
                    exit(errno);
                }
                close(output_fd);
            }
            
            //configure stdin from previous pipe (if not first command)
            if (i > 0) {
                if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) {
                    perror("dup2");
                    exit(errno);
                }
            }
            
            //configure stdout to next pipe (if not last command)
            if (i < clist->num - 1) {
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                    perror("dup2");
                    exit(errno);
                }
            }
            
            //close all pipe file descriptors in the child
            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            //execute command
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror(CMD_ERR_EXECUTE);
            exit(errno);
        }
    }
    
    // Parent process: close all pipe file descriptors
    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    //wait for child processes to complete
    int status;
    for (int i = 0; i < clist->num; i++) {
        waitpid(pids[i], &status, 0);
        if (i == clist->num - 1) {
            //save exit code of the last command in the pipeline for rc cmd
            last_command_exit_code = WEXITSTATUS(status);
        }
    }
    
    return OK;
}


//check for build in commands
Built_In_Cmds match_command(const char *input) {
    if (strcmp(input, EXIT_CMD) == 0) {
        return BI_CMD_EXIT;
    } else if (strcmp(input, "dragon") == 0) {
        return BI_CMD_DRAGON;
    } else if (strncmp(input, "cd", 2) == 0 && 
              (input[2] == '\0' || isspace((unsigned char)input[2]))) {
        return BI_CMD_CD;
    } else if (strcmp(input, "rc") == 0) {
        return BI_NOT_BI; //handling rc in exec_local_cmp_loop() 
    }
    return BI_NOT_BI;
}


//Execute a built in command
Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    if (cmd->argc == 0) {
        return BI_NOT_BI;
    }
    if (strcmp(cmd->argv[0], EXIT_CMD) == 0) {
        return BI_CMD_EXIT;
    } else if (strcmp(cmd->argv[0], "dragon") == 0) {
        print_dragon();
        return BI_EXECUTED;
    } else if (strcmp(cmd->argv[0], "cd") == 0) {
        if (cmd->argc == 1) {
            //no arguments
        } else if (cmd->argc >= 2) {
            if (chdir(cmd->argv[1]) != 0) {
                last_command_exit_code = errno;
            } else {
                last_command_exit_code = 0;
            }
        }
        return BI_EXECUTED;
    } else if (strcmp(cmd->argv[0], "rc") == 0) {
        printf("%d\n", last_command_exit_code);
        return BI_EXECUTED;
    }
    
    return BI_NOT_BI;
}


//Main command execution loop
int exec_local_cmd_loop() {
    char input[ARG_MAX];
    int rc = OK;
    
    // Read first command without a prompt
    if (fgets(input, sizeof(input), stdin) == NULL) {

        printf("%s", SH_PROMPT); // Print prompt before returning
        return OK;
    }
    
    // Process commands in a loop
    do {
        // Remove trailing newline
        input[strcspn(input, "\n")] = '\0';
        
        if (strlen(input) == 0) {
            printf(CMD_WARN_NO_CMD);
        } else {
            // Check for built-in commands first
            Built_In_Cmds cmd_type = match_command(input);
            
            if (cmd_type == BI_CMD_EXIT) {
                break; // Exit on exit command
            } else if (cmd_type == BI_CMD_DRAGON) {
                print_dragon();
            } else if (cmd_type == BI_CMD_CD) {
                cmd_buff_t cmd;
                parse(input, &cmd);
                if (cmd.argc >= 2) {
                    if (chdir(cmd.argv[1]) != 0) {
                        last_command_exit_code = errno;
                    } else {
                        last_command_exit_code = 0;
                    }
                }
                free(cmd._cmd_buffer);
            } else if (strcmp(input, "rc") == 0) {
                printf("%d\n", last_command_exit_code);
            } else {
                // For external commands
                command_list_t cmd_list = {0};
                rc = build_cmd_list(input, &cmd_list);
                
                if (rc == ERR_TOO_MANY_COMMANDS) {
                    printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
                } else if (cmd_list.num == 0) {
                    printf(CMD_WARN_NO_CMD);
                } else {
                    rc = execute_pipeline(&cmd_list);
                }
                
                free_cmd_list(&cmd_list);
            }
        }
        
        //print prompt after each command
        printf("%s", SH_PROMPT);
        fflush(stdout);
        
    } while (fgets(input, sizeof(input), stdin) != NULL);
    
    //print an additional prompt after EOF is detected
    printf("%s", SH_PROMPT);
    fflush(stdout);
    
    return rc;
}