This is a custom command-line interpreter that uses its own commands, while also
borrowing standard bash functionality so as to not lose any functionality. It also still
supports nested I/O redirection.

Custom commands:

       calc --- starts a calculator process. performs 2-number integer calculations. exit with 'exit' or Ctrl-D
       pwd --- prints the path of the current working directory
       cd [path]--- changes the path of the current working directory (relative or static)
       listf --- lists the contents of current working directory, similarly to 'ls'. available options: l, a, c, m
               (options a, c, and m all require option l to be specified)

**Makefile is included. Type 'make all' to generate executable.
