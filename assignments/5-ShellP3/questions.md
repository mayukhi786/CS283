1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

When forking child processes for a pipeline:
- The shell forks a process for each command.
- Each child process executes its respective command with execvp().
- The parent process (the shell) waits for all child processes to finish using waitpid() in a loop.
This ensures that the shell does not return to the prompt until all commands in the pipeline have completed execution.

If waitpid() is not called for all child processes, the shell does not collect the exit status of child processes, causing them to become zombies (defunct processes). Also, if the shell does not wait for children, it might start accepting user input while piped commands are still running in the background, leading to errors and unexpected outputs.

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

Closing unused pipe ends after dup2() is necessary to prevent resource leaks and deadlocks -- Open file descriptors consume system resources, and closing frees them. If a process keeps a write-end open but doesn’t write, the reader might block indefinitely waiting for input. And if the read-end remains open, the writer won’t receive an EOF signal, causing it to hang.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

cd is implemented as a built-in because each process has its own working directory.The child process cannot change the parent shell's directory. If cd were an external command, it would only change the directory in the child process, not the shell itself. The shell would remain in the same directory after the child exits.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

I would use:
- Dynamic memory allocation in cmd_buff_t instead of a fixed sized array
- Use dynamic allocation for pipe file descriptors instead of the fixed size array int pipes[CMD_MAX - 1][2]

Trade-offs:
- More memory usage/overhead from dynamic allocation
- May cause leaks if memory isn't freed carefully
