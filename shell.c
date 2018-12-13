// Jussi Moilanen
// Käyttöjärjestelmät ja systemiohjelmointi Harjoitustyö H2, 
// shell.c
// Lähteet: https://brennan.io/2015/01/16/write-a-shell-in-c/,
// Kurssin luentomateriaali,
// Stackoverflow

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#define error_message "An error has occurred\n"

int cd (char ** args);
int wish_exit (); 
char * path (char ** args, char *);
bool launch (char ** args);
void loop_shell();  
void batch(char filename[100]);
char **builtin_str();
bool execute(char **args);
// build in funktioiden määrittelyt
struct Builtin {
  char *name;
  int (*func) (char**);
};

struct Builtin builtins[] = { 
  { "cd", &cd },
  //{ "path", &path },
  { "exit", &wish_exit }
};

/*char **builtin_str() { 

  char **builtin = malloc(1024*sizeof(char));
  builtin[0] = "cd";
  builtin[1] = "path";
  builtin[2] = "wish_exit";
  return builtin;
}*/
int main(int argc, char **argv){ 
// suorittaa batch filen jos antaa 2 argumenttia, muuten ajaa shellin
	if (argc == 1){
		loop_shell();
	} else if (argc == 2) {
		batch(argv[1]);
	} else {
		printf("Too many commandline arguments.\n");	
	}
  	return 0;
}

char * readline(){ // getlinella luetaan, palautetaan rivi
	ssize_t size = 0;
  	char *line = NULL;
  	if (getline(&line, &size, stdin) == -1){
  		perror("getline");
  	}
	//printf("%c", line);
  	return line;
}

char ** parseline(char *line) { // jäsentää rivin ja palauttaa komennot (tokens), joita voidaan suorittaa

  	int bufsize = 64, position = 0;
  	char **tokens = malloc(bufsize * sizeof(char*));
  	char *token;
	char delimeters[10] = " \t\n\r\a"; //määritetään rajamerkit ja lisätään tokeniin

  	if (!tokens) {
    		fprintf(stderr, "lsh: allocation error\n");
    	exit(EXIT_FAILURE);
  	}
  	// otetaan ensimmöinen token
  	token = strtok(line, delimeters);
  	// Luupataan muiden tokenien läpi ja lisätään tokens
  	while (token != NULL) {
    		tokens[position] = token;
    		position++;
	
    	if (position >= bufsize) {
      		bufsize += 64;
      		tokens = realloc(tokens, bufsize * sizeof(char*));
      		if (!tokens) {
        		fprintf(stderr, "lsh: allocation error\n");
        		exit(EXIT_FAILURE);
      		}
    	}
    		token = strtok(NULL, delimeters); //rajaarvot \t\n\r\a tokeniin
  	}

  	tokens[position] = NULL;
  	return tokens;
}

// sisäänrakennetut komennot: cd ja exit
int cd (char ** args){ 
	if (args[1] == NULL) 
	{
		printf("wish: expected argument to \"cd\"\n");
	} 
	else {
		if (chdir(args[1]) != 0) {
			perror("wish: ");
		}
	}
	return 0;

}

int wish_exit (){
	exit(0);
}

/*char * path (char ** args, char* path) {
	int i;
	path = malloc(1024*sizeof(char));
	strcpy(path,"");
	char str[] = ":";
	if (args[1] == NULL){ return path;}// ei argumentteja

	for (i = 1; args[i] != NULL; i++) {
		strcat(path, args[i]);
		strcat(path, str);
	}
	return path;
} */

bool execute(char **args){

	//char **builtin = builtin_str();
	if (args[0] == NULL){
		return 1;
	}

	for (int i=0; i< 3; i++) { // 3 on builtin-komentojen määrä

		if (strcmp(args[0], builtins[i].name) == 0) {
      		return (*builtins[i].func)(args);
    	}
	}
	return launch(args);
}

// Ottaa listan argumentteja tokeneita joita suoritetaan
// Hyödynnetään fork() funktiota ja palautetaan boolean arvo
bool launch (char ** args) {
	
	pid_t wpid, pid = fork(); // luodaan lapsiprosessi
	int status;
	if (pid < 0) {
		perror("Error occurred.\n");
		
	} else if (pid == 0) {
		/*if (args[1] == ">") {
			freopen (args[2],"w",stdout);
  			execvp("ls", (char *[]){"ls", NULL});
  			fclose (stdout);
		}*/
		  	
		if (execvp(args[0], args) < 0) {
			perror("wish:\n");
			exit(EXIT_FAILURE);
		}
	} else {
		do { // isäntprosessi
      			wpid = waitpid(pid, &status, WUNTRACED);
    		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
  	}
	
	return true;
}

// Lukee, parsii ja suorittaa annetut komennot. 
void loop_shell(){
  	char *line;
  	char **args;
  	char *path = malloc(1024*sizeof(char));
  	strcpy(path,"/bin");

  	do {
    		printf("wish> ");
	    	line = readline(); //luku
    		args = parseline(line); // parse
    		execute(args); // suoritus
    		free(line);
    		free(args);
  	} while(true);
}

void batch(char filename[100]){ // lukee batchfilen ja suorittaa komennot
	printf("Opening %s.\n", filename);
	char line[100];
	char **args;
	FILE *file = fopen(filename, "r");
	if (file == NULL)
	{
		printf("File not found.\n");
		exit(1);
	} else {

		while(fgets(line, sizeof(line), file)!= NULL)
		{
			printf("\n%s", line);
			execute(parseline(line));
		}
		
	}
	fclose(file);
	free(args);
}
