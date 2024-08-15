#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <csignal>

const size_t size = getpagesize();


// Segmentation fault signal handler
void handleSegmentationFault(int signal) {
    std::cerr << "Segmentation fault (signal " << signal << ") detected!" << std::endl;
    std::cerr << "Exiting program" << std::endl;
    std::exit(EXIT_FAILURE);
}

int main()
{
    std::signal(SIGSEGV, handleSegmentationFault);
    
    void* addr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if(addr == MAP_FAILED){
        std::cerr << "mmap allocation failed" << std::endl;
        return 1;
    } 

    std::cout << "mmap allocated memory successfully @ address : "<< addr << std::endl;

    const char* message = "Hello, mmap";
    memcpy(addr, message, (strlen(message) + 1) );
    std::cout << "Data in memory: " << static_cast<char*>(addr) << std::endl;

    if(mprotect(addr, size, PROT_READ) == -1){
        std::cerr << "mprotect failed" << std::endl;
        munmap(addr,size);
        return 1;
    }

    std::cout << "Memory protection changed to read-only" << std::endl;

    // Attempt to write to the memory    
    memcpy(addr, "New data", 8);


    if(munmap(addr,size) == -1){
        std::cerr << "munmap failed" << std::endl;
        return 1;
    }

    std::cout << "Memory Successfully unmapped" << std::endl;

    return 0;
}