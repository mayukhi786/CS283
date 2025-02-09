1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  Firstly, fgets() is safe because it allows us to specify the maximum number of characters to read, which prevents buffer overflows. Also, fgets() reads an entire line, including spaces, until it gets to a newline (\n) or EOF. This is useful because the commands can contain spaces, and we need line-byline reading. Since fgets() reads a full line into a buffer, we can process the input before execution and skip whitespace, detect the exit command, etc.

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  A fixed-size array has a set size determined at compile-time. This means allocating a fixed amount of memory, and if the user enters a command that exceeds the buffer size (SH_CMD_MAX), it could lead to a buffer overflow and other issues. By using malloc(), I can allocate exactly the amount of memory needed for cmd_buff. I still allocate enough space for the maximum allowed input (SH_CMD_MAX), but this allows more flexibility to adapt the memory allocation for different situations if needed. I can dynamically allocate more memory if needed and free it once it's no longer necessary, ensuring efficient memory usage.


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  Trimming spaces prevents pmpty or invalid commands. Leading spaces can cause commands to be misinterpreted or fail to execute. When matching the command to execute some logic, we need the actual command entered, without leading and trailing spaces. Not trimming them will lead to inaccurate string matching and the necessary logic can't be executed. It also avoids unintended empty commands in pipelining, as extra spaces may interfere with parsing and storing commands.


4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  1. Output Redirection (> and >>) - The > operator redirects a command’s output to a file, overwriting any existing content. The >> operator appends output to a file without overwriting previous data.
    > 2. Input Redirection (<) - The < operator redirects a file’s contents to a command as if the user typed it in manually.
    > 3. Redirecting Both STDOUT and STDERR (>& or 2>) - 2> redirects STDERR to a file.
    2>&1 makes STDERR point to the same location as STDOUT.

    Challenges while implementing these include proper file handling, replacing STOUT/STDIN (depending on input/output redirection), proper error handling, order of redirection, etc.
    

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  Redirections redirect input or output to/from files or devices instead of the default terminal, while piping passes the output of one command as the input to another command, allowing commands to be chained together in a pipeline. Redirection uses uses < (for input redirection), > (for output redirection), and >> (for appending to a file), and piping uses the pipe symbol | to connect multiple commands.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  By separating regular output from error messages, we can distinguish between the successful output of the program and any errors that occurred. This makes it simpler to handle errors in a structured way, such as logging or displaying specific error messages without interfering with normal output. STDOUT is also usually redirected to files or piped into other commands for further processing. If STDERR were mixed in with regular output, they would be included in these operations and mess up the expected output.

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  In cases where a command outputs both STDOUT and STDERR, it’s mostly good to keep them separate so that users can easily distinguish between regular output and error messages. But merging STDOUT and STDERR might be useful when we want a unified log of everything that happens during a command execution, including both successful output and error messages.
    
    To merge, we could implement an option (e.g., a flag or environment variable) that allows users to merge STDOUT and STDERR. The typical way to merge them in Unix-like systems is by redirecting STDERR to STDOUT, using syntax like: command 2>&1