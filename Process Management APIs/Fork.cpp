#include <iostream>
#include <unistd.h>

int main() {
    pid_t pid = fork();
    
    if(fork < 0){
        std::cerr << "Fork Failed" << std::endl;
        return 1;
    }
    else if (pid == 0){
        std::cout << "In Child Process with PID : " << getpid() << std::endl;
    }
    else{
        std::cout << "In Parent Process with PID : " << getpid() << std::endl;
        std::cout << "Child Process PID : " << pid << std::endl; 
    }
    return 0;
}       