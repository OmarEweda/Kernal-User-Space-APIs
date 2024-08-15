#include <iostream>
#include <sys/mman.h>  // For mlock, munlock
#include <cstring>     // For memset
#include <unistd.h>    // For getpagesize

const size_t size = getpagesize();


int main()
{  
    void* addr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if(addr == MAP_FAILED){
        std::cerr << "mmap allocation failed" << std::endl;
        return 1;
    } 

    std::cout << "mmap allocated memory successfully @ address : "<< addr << std::endl;

    if(mlock(addr, size) == -1){
        std::cerr << "mlock failed" << std::endl;
        munmap(addr,size);
        return 1;
    }
    
    std::cout << "Memory Locked in RAM" << std::endl;

    memset(addr, 0, size);

    if(munlock(addr, size) == -1){
        std::cerr << "munlock failed" << std::endl;
        munmap(addr,size);
        return 1;
    }


    std::cout << "Memory Unlocked" << std::endl;

    if(munmap(addr,size) == -1){
        std::cerr << "munmap failed" << std::endl;
        return 1;
    }

    std::cout << "Memory Successfully unmapped" << std::endl;

    return 0;
}