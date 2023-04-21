# Project 1
## Command Data Structure Design
We implemented the command data structures by focusing on storing the arguments needed for the `execvp()` functions which is the command and the string array of the command and the command arguments.
The commands also serve as nodes in the linked list similar to how the commands are stored in project0.
However we also made a `list` struct that would have all the nodes inside it and this struct would act as the entire linked list that the nodes will be stored inside.

## Functions
Most of the functions are basic helper functions that return values in the `list` or `node` structures so they're pretty self-explanatory since they basically just return one line. 