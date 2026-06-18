#include<stdio.h>
#include<unistd.h>
#include<Windows.h>

#define MAX_INPUT 1024
#define MAX_ARGS 64

int handle_input(char *input, char **args) {
    int idx = 0;
    printf("Handle input successful\n");
    args[idx] = strtok(input, " \t\r\n");
    
    while(args[idx] != NULL && idx < MAX_ARGS - 1) {
        args[++idx] = strtok(NULL, " \t\r\n");
    }
    args[idx] = NULL;
    return idx;
}

void handle_external_command(char *command) {
    
}

int main() {
    char input[MAX_INPUT]; 
    char *args[MAX_ARGS];

    while(1) {
        char cwd[256];
        getcwd(cwd, sizeof(cwd));
        printf("%s $ ", cwd);
        fflush(stdout);

        char *get_input = fgets(input, MAX_INPUT, stdin);
        if(get_input == NULL) {
            perror("get input failed");
            continue;
        }
        get_input[strcspn(get_input, "\t\r\n")] = '\0';
        
        int idx = handle_input(input, args);
        for(int i = 0; i < idx; i++) {
            printf("%s ", args[i]);
        }
        printf("\n");
        // Handle user request
    }

    return 0;
}

// cd "D:\Truong Nguyen Binh Minh\code\New folder\src"