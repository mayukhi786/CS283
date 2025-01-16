#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
//add additional prototypes here
void reverse_string(char *, int);
void word_print(char *, int);


int setup_buff(char *buff, char *user_str, int len){
    //TODO: #4:  Implement the setup buff as per the directions

    // Checking if the provided pointers are NULL
    if (buff == NULL || user_str == NULL) {
        // -2 error: Either the buffer or the user string pointer is NULL
        return -2;
    }

    // Initialize pointers for input string and buffer
    char *src = user_str;
    char *dst = buff;
    int char_count = 0;      // Counter for non-whitespace characters in the string
    int consecutive_space = 0; // To track consecutive whitespace

    // Traverse the string
    while (*src != '\0') {
        // Check if character is a whitespace (space or tab)
        if (*src == ' ' || *src == '\t') {
            if (!consecutive_space) {
                *dst = ' ';            // Add a single space if no consecutive space has been added yet
                dst++;                 // Move buffer pointer forward
                char_count++;          // Increment the character count
                consecutive_space = 1; // Mark that we  added a space
            }
        } else {
            // Non-whitespace character
            *dst = *src;              //Put character in the buffer
            dst++;                    // Move buffer pointer forward
            char_count++;             // Increment the character count
            consecutive_space = 0;    // Reset consecutive space flag
        }

        // If the buffer is full, return an error
        if (char_count > len) {
            return -1; // Error: user string is too large
        }

        src++; // Move to the next character in the string
    }

    // Fill the remaining buffer with '.' characters
    for(int i = char_count; i < len; i++) {
        *dst++ = '.';     // Add a dot to the buffer          
    }

    return char_count; // Return the length of the string

}

void print_buff(char *buff, int len){
    printf("Buffer:  ");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    putchar('\n');
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}

int count_words(char *buff, int len, int str_len){
    //YOU MUST IMPLEMENT
    (void)len; //Marking len as unused, to prevent warnings

    char *ptr = buff; //Pointer for buffer
    int count = 0;   // initialize counter
    int in_word = 0; //to check if character is in a word

    // Traversing the string
    for(int i = 0; i < str_len; i++){
        if(*ptr != ' '){    //Check if character isn't a whitespace
            if(!in_word){   //Check if character is in a word
                count++;    //New word
                in_word = 1; //Mark as in word
            }
        } else {
            in_word = 0;   //Reset in word marker
        }
        ptr++;
    }

    return count; //Return count of words
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS

void reverse_string(char *buff, int str_len){
    char *start = buff; //Pointer for start of buffer
    char *end = buff + str_len - 1; //Pointer at end

    // Swap characters from start to end
    while(start < end){
        char temp = *start;
        *start = *end;
        *end = temp;

        start++;
        end--;
    }

    // Print the reversed string
    printf("Reversed String: ");
    for(int i = 0; i < str_len; i++){
        putchar(buff[i]);
    }
    printf("\n"); //Print newline
}

void word_print(char *buff, int str_len){
    char *start = buff; //Pointer for start of buffer
    char *end; //Initializing end pointer for end of words
    int word_count = 1; //Initialize word counter

    printf("Word Count\n----------\n");
    while (*start != '.' && (start - buff) < str_len) {
        // Skip leading spaces
        while (*start == ' ' && (start - buff) < str_len) {
            start++;
        }
        //Traversing a word
        if (*start != ' ' && *start != '.' && *start != '\0') {
            end = start;

            // Find the end of the word
            while (*end != ' ' && *end != '.' && *end != '\0') {
                end++;
            }

            // Calculate word length
            int word_len = end - start;

            // Print the word number, word, and length
            printf("%d. ", word_count++);
            for (int i = 0; i < word_len; i++) {
                putchar(*(start + i));
            }
            printf(" (%d)\n", word_len);

            // Move start to end for the next word
            start = end;
        }
    }    
}

int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //      (argc < 2) checks that there was at least 1 command line argument provided
    //      if argv[1] doesn't exist, the first part of the if condition will be true
    //      (because argc in that case is < 2), and the program exits immediately.
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);  //Command line error
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0); //Success
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    //      The if statement checks if there are less than 3 arguments provided, i.e,
    //      the string argument isn't provided. If the string is missing, program exits.
    //      The check makes sure that the third argument exists before referencing argv[2].
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    buff = malloc(BUFFER_SZ * sizeof(char));
    if (!buff) {
        printf("Error: Memory allocation failed.\n");
        exit(99);
    }

    //Getting length of user's string
    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos

    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2); //Memory allocation problem
            }
            printf("Word Count: %d\n", rc);
            break;

        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        case 'w':
            word_print(buff, user_str_len); //Call the required function
            break;
        
        case 'r':
            reverse_string(buff, user_str_len); //Call the required function
            break;
        
        case 'x':
            // checking if there are 5 arguments in total (3 after -x)
            if (argc < 5){
                usage(argv[0]);
                exit(1);
            }
            printf("Not implemented!\n");
            exit(3); //exit with error code

        default:
            usage(argv[0]);
            exit(1); //Command line problem
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff,BUFFER_SZ);
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//          It's good practise to provide both the pointer and length because
//          the size of the buffer being explicitly passed prevents memory overflows
//          and other errors by not letting the functions write/read 
//          over the bounds of the buffer. It also makes the code more reusable,
//          because we can use the same functions with a different buffer length.