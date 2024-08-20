#include <iostream>
#include <unistd.h>
#include <wait.h>

int main() {
    pid_t pid = fork();
    
    if(pid < 0){
        std::cerr << "Fork Failed" << std::endl;
        return 1;
    }
    else if (pid == 0){
        std::cout << "In Child Process with PID : " << getpid() << std::endl;
        execl("/bin/ls", "ls" ,"-alih",NULL);
        /*
        char* args [] = {"ls","-alih",NULL};
        execv("/bin/ls", args);
        */
       std::cerr << "exe failed " << std::endl;
    }
    else{
        wait(NULL);
        std::cout << "Child Process finished" << std::endl; 
    }
    return 0;
}       