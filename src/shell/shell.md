### Simple shell implementation


##### Feature Support:

- execute single command

- pipe (|)

- redirection (< or >)

##### Implementationwork
1) scan_cmd - simple scan from keyboard to buffer

2) In new process call run_cmd(command *cmd);

3) parse_command returns command *. Within itself it calls parse_line. After parse_line returns, parse_command verify parse is done using peek function 

4) parse_line returns command *. Inside function we start parse calling parse_pipe function. 

5) parse_pipe returns command *. First it calls parse_exec. Then it checks next symbol after first
parse_exec, and if symbol equals "|" it calls parse_exec again. and pass results to function
command_create_pipe.

6) parse_exec returns command *. This is where all the parse is doing:
```
while (symbol != '|') {
	read_word();
	
	copy_word_to_command_argv();
	parse_redirects()
}
```

7) parse_redirects returns command *
```
while (symbol == '< or >')
	tok = gettoken();
	switch (tok)
		case Input
			...
		case Output 
			...
```