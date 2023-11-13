#include "load.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int main(int argc, const char * argv[], const char * envp[]){
    if(argc < 2){
        printf("Usage: %s <program>", argv[0]);
        return -1;
    }

    user_execve(argc - 1, argv + 1, envp);

}