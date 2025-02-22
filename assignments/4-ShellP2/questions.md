1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  We use fork() with execvp() because execvp() replaces the current process image. Without fork(), the shell would terminate after running a command. fork() creates a child process that runs the command, while the parent shell waits using waitpid() and continues afterward. This ensures maintains the current state of the shell, and allows error handling without affecting the shell.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  f fork() fails, it returns a negative value usually -1, indicating that the child process wasn't created. In my implementation, I check for this negative value and print an error message using perror("Unsuccessful fork") and free cmd._cmd_buffer to release allocated memory. 

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  execvp() searches for the command in directories listed in the PATH environment variable. It checks each directory in PATH until it finds an executable with the given name or returns an error if not found. The PATH system environment variable plays a role in this process

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  wait() ensures the parent process waits for the child to finish and collects its exit status, preventing zombie processes. Without wait(), the child process would become a zombie causing clutter or errors until the parent terminates

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**:  WEXITSTATUS() is a macro used to extract the exit status of a child process that was created using fork() and has completed execution. This exit status is provided by the child process when it exits and is used to indicate whether the process completed successfully or encountered an error. WEXITSTATUS() is used after the child process has completed to get the exit code from the child and assign it to the last_command_exit_code variable. This helps the parent process track the success or failure of commands executed in the child process.

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  My function handles quoted arguments by detecting quotes, skipping them, and capturing everything inside as a single argument. This ensures that spaces inside quotes don't split the argument into multiple parts. 
    > This is necessary because spaces should not be treated as argument separators; spaces inside quotes shouldn't cause the argument to break into separate strings. Multi-word arguments and arguments with paths/spaces should be parsed properly.


7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  My new parsing logic works on a single cmd_buff, and handles quotes and spaces in the arguments instead of splitting the commands by pipes. 
    >The challenge was to properly ensure that quoted arguments are correctly parsed and not split by spaces, and changing the parsing logic to handle a single cmd_buff_t instead of the command_list_t.

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**: In a Linux system, signals are a form of asynchronous inter-process communication (IPC) used to notify a process about an event, like a user input, system error, or termination request, essentially acting as a quick interruption to a process to alert it about something that needs immediate attention; unlike other IPC methods, signals are primarily used for simple, event-driven notifications rather than transferring large amounts of data between processes. 

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  SIGINT (Signal Interrupt): This signal is typically sent when a user presses Ctrl+C in the terminal, essentially interrupting a running process and giving it a chance to gracefully terminate; it's used to quickly stop a program when the user wants to exit immediately. 

    > SIGTERM (Signal Terminate): This is the standard signal for requesting a process to gracefully terminate, allowing the program to perform any necessary cleanup tasks before exiting; it's the preferred method for ending a process when possible.
     
    > SIGKILL (Signal Kill): Considered a "forceful" termination signal, SIGKILL immediately stops a process without giving it the opportunity to clean up; it's used as a last resort when a process is unresponsive to other signals or needs to be terminated immediately. 

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  When a process receives a SIGSTOP signal, it is immediately suspended and cannot continue execution until it receives a SIGCONT signal to resume.
    > It cannot be caught or ignored like SIGINT because SIGSTOP is designed to forcefully stop a process without allowing the process to perform any cleanup actions, like acting as a pause button at the kernel level.