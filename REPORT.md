#Project 1
##Command Data Structure Design
We implemented the command data structures by focusing on storing the arguments needed for the `execvp()` functions which is the command and the string array of the command and the command arguments.
The commands also serve as nodes in the linked list similar to how the commands are stored in project0.
However we also made a `list` struct that would have all the nodes inside it and this struct would act as the entire linked list that the nodes will be stored inside.

## Functions
Most of the functions are basic helper functions that return values in the `list` or `node` structures so they're pretty self-explanatory since they basically just return one line. `createNode` and `createList` initialize their respective structure variables and allocate memory for the variables that require memory allocation.

## Piping and Command Execution
The piping function we have is actually able to handle both single and piped commands so therefore in the fork+exec+wait structure we have the `pipeline()` function executed with the list of commands entered into the shell command line. For the built-in commands like `cd`, `pwd`, and `set` the parent process will handle them and execute them. The piping logic is essentially the same as the piping example in that `command1` executes and the output is fed into the reading end of the pipe and used by `command2` which then outputs to the write end. Then it'll just take the next command in the pipe, and repeat the process.

## Resources
Most used resource is really the manual pages for the functions. A lot of the conceptual implementations are all found in the lecture slides and from the class materials such as the professors example codes. 