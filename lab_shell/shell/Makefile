build-shell: main.c eval.c completion.c helper.c redirection.c
	gcc -o shell main.c eval.c completion.c helper.c redirection.c -lreadline

run-shell: shell
	./shell

clean:
	rm -f shell