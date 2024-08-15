#include <iostream>
#include <sys/mman.h>  // For mmap, munmap
#include <fcntl.h>     // For file descriptors
#include <unistd.h>    // For close
#include <cstring>     // For memcpy

const size_t size = 4096;

int main()
{
    void* addr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if(addr == MAP_FAILED){
        std::cerr << "mmap allocation failed" << std::endl;
        return 1;
    } 

    std::cout << "mmap allocated memory successfully @ address : "<< addr << std::endl;

    const char* message = "Hello, mmap";
    
    memcpy(addr, message, (strlen(message) + 1) );

    std::cout << "Data in memory: " << static_cast<char*>(addr) << std::endl;

    if(munmap(addr,size) == -1){
        std::cerr << "munmap failed" << std::endl;
        return 1;
    }

    std::cout << "Memory Successfully unmapped" << std::endl;

    return 0;
}