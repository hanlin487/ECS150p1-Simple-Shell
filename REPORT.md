# Project 1
## Command Data Structure Design
We implemented the command data structures by focusing on storing the arguments needed for the `execvp()` functions first which is the command and the string array of the command and the command arguments. This is done by simply storing the commands in the string array `command` that is in the `node` struct
The commands also serve as nodes in the linked list similar to how the commands are stored in project0.
However we also made a `list` struct that would have all the nodes inside it and this struct would act as the entire linked list that the nodes will be stored inside. The list doesn't really serve much of a purpose other than to contain the nodes/commands that the user will input

## Functions
Most of the functions are basic helper functions that return values in the `list` or `node` structures like values in the command array, the amount of commands/the lenght of the command array, the head and tail of the linked list of commands, etc so they're pretty self-explanatory since they basically just return one line. `createNode` and `createList` initialize their respective structure variables and allocate memory for the variables that require memory allocation. The only complex function is `parse` which does a lot of the heavy lifting of the program. 

The parsing works by using `strok` and delimiting by a space `" "` in order to find the commands from the shell's command line. In the case of piping it actually doesn't separate the piped commands in the function itself but actually does the parsing in the main function so as to not complicate the original parsing function. The parsing method in the main function for when parsing is present is similar to that of the regualr parsing however it instead will go through and search for the `|` pipe symbol and store each command in the pipe as its own object before moving on to other commands.

## Piping and Command Execution
The piping function we have is actually able to handle both single and piped commands so therefore in the fork+exec+wait structure we have the `pipeline()` function executed with the list of commands entered into the shell command line. Therefore whether the command is something simple like `ls` or `ls -l | sort` the `pipeline()` function will be able to handle both. For the built-in commands like `cd`, `pwd`, and `set` the parent process will handle them and execute them. The piping logic is essentially the same as the piping example in that `command1` executes and the output is fed into the reading end of the pipe and used by `command2` which then outputs to the write end. Then it'll just take the next command in the pipe, and repeat the process until all commands have gone through the pipe and there are no more commands to run.

## Testing
Lots of testing was seeing the example outputs detailed in the projects assignment and getting our own shell to emulate those. Then for the other remaining cases the shell was tested mainly with the simple testing script supplied.

## Resources
Most used resource is really the manual pages for the functions. A lot of the conceptual implementations are all found in the lecture slides and from the class materials such as the professors example codes. The GNU docs provided in the `project1.html` page were also utilized to implement the more complicated processes like parsing and piping.