#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <ctype.h>

//list of paths
char **paths = NULL;
//number of paths
int pathCount = 0;

bool isPathDeleted = false;

char* stringformat(char *s){
	char* word = NULL;
	char* token = strtok(s, "\t\n\r\f\v");
	if (token != NULL){
		word = strdup(token);
	}
	return word;
}

//pwd process
void processpwd(char* line){
	char cwd[1024];
	if (getcwd(cwd, sizeof(cwd)) != NULL){
		printf("%s\n", cwd);
	}
	else{

		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
	}
}

//cd process
void processcd(char *line){

	char *path = stringformat(line);
				
	int ret = chdir(path);

			
	
	if (ret == 0){
		chdir(path);

	}
	//if this command was unsuccessful then it will print an error message
	else if (ret == -1){
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
		
	}
	//this is to catch any unexpected errors
	else{
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
		
	}

}

//handle ls command
void handlels(int wordCount, char **words){

	if (wordCount > 1){
	char *path = stringformat(words[1]);
	char *lsArgs[] = {"ls", path, NULL};
	execv("/usr/bin/ls", lsArgs);
	} else{
		char *lsArgs[] = {"ls", NULL};
	execv("/usr/bin/ls", lsArgs);
	}
	
}

//handle echo command
void handleEcho(char **words, int wordCount){
	char *word = NULL;
	for (int i = 1; i < wordCount; i++){
		if (i == 1){
			word = stringformat(words[i]);
		}
		else{
			char *temp = stringformat(words[i]);
			word = strcat(word, " ");
			word = strcat(word, temp);
		}
	}
	if (word == NULL){
		word = "";
	}

	char *echoArgs[] = {"echo", word, NULL};
	execv("/bin/echo", echoArgs);
}

//handle path command
void addPathstoGlobalVariable(char **words, int wordCount){
	//get current path using getcwd
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	if (wordCount == 1){
		paths = realloc(paths, (pathCount+1) * sizeof(char *));
		paths[pathCount] = cwd;
		pathCount++;
		
	}
    //add cwd to each of the global paths variable
	for (int i = 1; i < wordCount; i++){
		char *path = cwd;

		//reallocate memory for paths
		paths = realloc(paths, (pathCount+1) * sizeof(char *));
		//add path as a string to paths
		paths[pathCount] = path;
		pathCount++;
		//print the path
		
	}
	//concatenate path in words to each paths
	for (int i = 0; i < pathCount; i++){
		char *path = paths[i];
		char *temp = stringformat(words[i+1]);
		if (strstr("/",words[i+1]) == NULL){
			temp = strcat(words[i+1], "/");
		}
		//concatenate path to paths
		char *commandPath = strcat(path, "/");
		commandPath = strcat(commandPath, temp);
		//add commandPath to paths
		paths = realloc(paths, (pathCount+1) * sizeof(char *));
		paths[pathCount] = commandPath;	

	}
	
}



//handle the cat command
void handleCatCommand(char *file){
	file = stringformat(file);
	int pid = fork();
	if (pid == 0){
		char *catArgs[] = {"cat", file, NULL};
		execv("/bin/cat", catArgs);
		exit(EXIT_SUCCESS);
	}
	else{
		int status;
		wait(&status);
	}

}

//handle the rm command with -r or -f prefix
void handleRmCommand(char *file, char *prefix){
	file = stringformat(file);
	prefix = stringformat(prefix);
	int pid = fork();
	if (pid == 0){
		char *rmArgs[] = {"rm", prefix, file, NULL};
		execv("/bin/rm", rmArgs);
		exit(EXIT_SUCCESS);
	}
	else{
		int status;
		wait(&status);
	}
}



void handlePathCommand(char *command){

	for (int i = 0; i < pathCount; i++){
		char *path = paths[i];
		
		path = stringformat(path);
		//concatenate command to path
		path = strcat(path, command);
		//check if commandPath is a valid path
		if (access(path, X_OK) == 0){
			//execute commandPath
			char *commandArgs[] = {path, NULL};
			execv(path, commandArgs);
			exit(EXIT_SUCCESS);
		}
	}


	//if command is not found in any of the paths, print error message
	char error_message[30] = "An error has occurred\n";
	write(STDERR_FILENO, error_message, strlen(error_message));
	
}


void process(char *line){

	

	char *token = strtok(line," ");
	char **words = NULL;
	int wordCount = 0;

	char *command = NULL;

	while(token != NULL){
			char *newWord = strdup(token);

			words = realloc(words, (wordCount+1) * sizeof(char *));

			words[wordCount] = newWord;

			token = strtok(NULL, " ");
			wordCount++;
			
	}

		if (isPathDeleted){
			if (strcmp(stringformat(words[0]),"cd") != 0 && strcmp(stringformat(words[0]),"exit") != 0 && strcmp(stringformat(words[0]),"path")){
				char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message));
			
			}else{
				if (strcmp(line, "exit\n") == 0 || feof(stdin)){
				
				exit(0);

				}

			else if (strcmp(stringformat(words[0]),"exit")==0 && wordCount > 1){
				char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message));
				}

			//this one will send an error message for an empty cd command and for cd with more than 1 argument
			else if ((strcmp(stringformat(words[0]),"cd") == 0 && wordCount == 1) || (strcmp(words[0],"cd")== 0 && wordCount > 2)){
				char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message));
				exit(EXIT_SUCCESS);
			}

			//now we are checking if the application can change directory to other places
			else if (strcmp(words[0],"cd")==0){
				
				processcd(words[1]);
				
			}

			else if (strcmp(stringformat(words[0]), "path") == 0 && wordCount > 1){
				isPathDeleted = false;
				addPathstoGlobalVariable(words, wordCount);
				
			}



			}
		}else{


			//these commands are executed if the path is not deleted.

			if (strstr(stringformat(line), ".sh") != NULL){
			if (paths == NULL){
				char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message));
			}else{
				int pid = fork();
				if (pid == 0){
					handlePathCommand(stringformat(line));
				}else{
					int status;
					wait(&status);
					if (strcmp(stringformat(line), "exit\n")==0 || feof(stdin)){
						exit(0);
					}
				}
				
			}
			
		}
		
			//checks if the command exit is called and exits the program or cntrl + D
		if (strcmp(stringformat(line), "exit") == 0 && wordCount == 1 || feof(stdin)){
				
				exit(0);

		}

		else if (strcmp(stringformat(words[0]),"exit") ==0 && wordCount > 1){
				char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message));
		}

			//checks if the command pwd is called and prints the current working directory
		else if (strcmp(stringformat(line), "pwd") == 0){
				processpwd(line);
			}

			//this one will send an error message for an empty cd command and for cd with more than 1 argument
		else if ((strcmp(stringformat(line),"cd") == 0 && wordCount == 1) || (strcmp(words[0],"cd")== 0 && wordCount > 2)){
				char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message));
				exit(EXIT_SUCCESS);
			}

			//now we are checking if the application can change directory to other places
		else if (strcmp(stringformat(words[0]),"cd")==0){
				
				processcd(words[1]);
				
			}

		else if (strcmp(words[0],"echo")==0){
				int pid = fork();

				if (pid == 0){

					handleEcho(words, wordCount);

				} else if (pid > 0){
				int status;
				wait(&status);
				if (strcmp(line, "exit\n")==0 || feof(stdin)){
					exit(0);
				}
			
				}
			}

			else if (strcmp(stringformat(words[0]), "ls") == 0){

				int pid = fork();

				if (pid == 0){

					handlels(wordCount, words);

				} else if (pid > 0){
				int status;
				wait(&status);
				if (strcmp(stringformat(line), "exit")==0 || feof(stdin)){
					exit(0);
				}
			
				}
				
			}

			//solution for path
			else if (strcmp(stringformat(words[0]), "path") == 0 && wordCount == 1 ){
				free(paths);
				paths = NULL;
				isPathDeleted = true;
				
			}

			else if (strcmp(stringformat(words[0]), "path") == 0 && wordCount > 1){
				isPathDeleted = false;
				addPathstoGlobalVariable(words, wordCount);
				
			}

			//handles cat as a command
			else if (strcmp(stringformat(words[0]), "cat") == 0 && wordCount >1){
				handleCatCommand(words[1]);
			}
			else if (strcmp(stringformat(words[0]), "cat") == 0 && wordCount == 1){
				char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message));
			}

			else if (strcmp(stringformat(words[0]), "rm") == 0 && wordCount > 2){
				handleRmCommand(words[2], words[1]);
			}

			else if (strcmp(stringformat(words[0]), "rm") == 0 && wordCount == 2){
				handleRmCommand(words[1], "");
			}



			//freeing memory
			for (int i = 0; i < wordCount; i++){
			free(words[i]);
			
			}
			free(words);
			
		}

}


//handle redirection
void handleRedirection(char* word){
	char **container = NULL;
	int containerCount = 0;
	word = stringformat(word);
	char *token = strtok(word, ">");

	while (token != NULL){
		char *newWord = strdup(token);
		container = realloc(container, (containerCount+1) * sizeof(char *));
		container[containerCount] = newWord;
		token = strtok(NULL, " ");
		containerCount++;
	}

	if (containerCount > 2 || containerCount == 1){
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
	}
	else{
		char* commandToProcess = stringformat(container[0]);
		char* fileToWrite = stringformat(container[1]);
		
		//check if file exists otherwise create it
		int fd = open(fileToWrite, O_CREAT | O_WRONLY | O_TRUNC, 0644);
		if (fd == -1){
			char error_message[30] = "An error has occurred\n";
			write(STDERR_FILENO, error_message, strlen(error_message));
		}
		else{

			//fork process
			int pid = fork();

			if (pid == 0){
				//child process
				//redirect stdout to file
				dup2(fd, STDOUT_FILENO);

				//process command
				process(commandToProcess);

				//close file
				close(fd);
				
				exit(EXIT_SUCCESS);
			}
			else{
				//parent process
				int status;
				wait(&status);
			}
		}
	}
}


//handle parallel commands
void handleParallelCommands(char *word){
	char **container = NULL;
	int containerCount = 0;
	word = stringformat(word);

	char *token = strtok(word, "&");

	while (token != NULL){
		char *newWord = strdup(token);
		container = realloc(container, (containerCount+1) * sizeof(char *));
		container[containerCount] = newWord;
		token = strtok(NULL, " &");
		containerCount++;
	}
	
	for (int i = 0; i < containerCount; i++){
		char *command = stringformat(container[i]);
		int pid = fork();

		if (pid == 0){
			if (strstr(command,">") != NULL){
				handleRedirection(command);
			}
			else if (strcmp(command, "\n")==0){
				continue;
			}else{
				process(command);
			}
				free(container[i]);
				exit(EXIT_SUCCESS);
			}
			else{
				int status;
				wait(&status);
		}
	}
	//free memory
	
}



//handles file input
void handleFileInput(char* file){
	
	char** filepaths = NULL;

	FILE *fp = fopen(file, "r");
		if (fp == NULL){
			char error_message[30] = "An error has occurred\n";
			write(STDERR_FILENO, error_message, strlen(error_message));
			exit(EXIT_FAILURE);
		}else{
			char *line = NULL;
			size_t len = 0;
			ssize_t read;
			while ((read = getline(&line, &len, fp)) != -1){
				if (strstr(line,"&") != NULL){
				handleParallelCommands(line);
					
				}
				else if (strstr(line,">") != NULL){
					
					handleRedirection(line);
				}
				else if (strcmp(line,"\n")==0){

				}
				else{

					line = stringformat(line);
					process(line);
				}
				
			}
			
			fclose(fp);
		}
}


int main(int MainArgc, char *MainArgv[]){

	bool isInteractive = false;
	
	if (MainArgc == 1){
		isInteractive = true;
	}else if(MainArgc > 2){
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(EXIT_FAILURE);
	}
	
	else{

		//open the file in *mainArgv
		char *file = MainArgv[1];
		handleFileInput(file);
	}

	while(isInteractive){
		printf("witsshell> ");

		//stores input value
		char *line = NULL;

		//stores length of input value
		size_t len = 0;

		ssize_t read;

		read = getline(&line, &len, stdin);
		
		if (strstr(line,">") != NULL){
			handleParallelCommands(line);
		}
		else if (strstr(line,"&") != NULL){
			
			handleRedirection(line);
		}
		else if (strcmp(line, "\n")==0){
			continue;
		}else{
			process(line);
		}
		
		free(line);
			
		
		

		
		

		
	}
	return 0;

}
