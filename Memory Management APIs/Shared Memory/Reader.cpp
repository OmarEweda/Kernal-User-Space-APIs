#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstring>


int main()
{
    key_t key = ftok("shmfile", 65);
    if(key == -1){
        std::cerr << "ftok failed" << std::endl;
        return 1;
    }

    int shmid = shmget(key, 1024, 0666 | IPC_CREAT);
    if(shmid == -1){
        std::cerr << "shmget failed" << std::endl;
        return 1;
    }

    char* str = static_cast<char*> (shmat (shmid, nullptr, 0));
    if(*str == static_cast<char>(-1)){
        std::cerr << "shmat failed" << std::endl;
        return 1;
    }

    std::cout << "Data read from shared Memory : " << str <<std::endl;

    if(shmdt(str) == -1){
        std::cerr << "shmdt failed" << std::endl;
        return 1;
    }


    if(shmctl(shmid,IPC_RMID, nullptr) == -1){
        std::cerr << "shmctl failed" << std::endl;
        return 1;
    }

    std::cerr << "shared memory deleted" << std::endl; 

    return 0;
}