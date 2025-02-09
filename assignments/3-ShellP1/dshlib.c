#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    memset(clist, 0, sizeof(command_list_t)); // Initialize clist

    char *cmd_token = strtok(cmd_line, PIPE_STRING); // Split on pipes '|'

    while (cmd_token != NULL)
    {
        while (*cmd_token == ' ') cmd_token++; // Trim leading spaces

        if (clist->num >= CMD_MAX)
        {
            return ERR_TOO_MANY_COMMANDS;
        }

        // Find first space (separates executable from arguments)
        char *space = strchr(cmd_token, ' ');
        if (space != NULL)
        {
            *space = '\0';  // Terminate the executable part
            space++;  // Move to arguments
            while (*space == ' ') space++;  // Skip extra spaces
        }

        // Check if executable name is too long
        if (strlen(cmd_token) >= EXE_MAX)
        {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        // Store executable
        strncpy(clist->commands[clist->num].exe, cmd_token, EXE_MAX - 1);
        clist->commands[clist->num].exe[EXE_MAX - 1] = '\0';

        // Store arguments (if any)
        if (space != NULL && *space != '\0')
        {
            if (strlen(space) >= ARG_MAX)  // Check if args exceed limit
            {
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            strncpy(clist->commands[clist->num].args, space, ARG_MAX - 1);
            clist->commands[clist->num].args[ARG_MAX - 1] = '\0';
        }
        else
        {
            clist->commands[clist->num].args[0] = '\0'; // No arguments
        }

        clist->num++;  // Increment command count
        cmd_token = strtok(NULL, PIPE_STRING); // Get next piped command
    }

    // Return based on if commands are found
    if (clist->num > 0) {
        return OK;
    } else {
        return WARN_NO_CMDS;
    }
}
