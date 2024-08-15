#include <iostream>
#include <cstdlib>

int main()
{   

    // Using malloc
    int* ptr = (int*) malloc (sizeof(int) * 5);
    if(!ptr){
        std::cerr << "malloc Memory Allocation failed"<< std::endl;
        return 1;
    }

    for (int i = 0; i < 5; ++i) {
        ptr[i] = i * 2;
    }

    for (int i = 0; i < 5; ++i) {
        std::cout << "Ptr ["<< i << "] =" << ptr[i] << std::endl;
    }

    //Using calloc
    int* cptr = (int*) calloc(5, sizeof(int));

    if(!cptr){
        std::cerr << "calloc Memory Allocation failed"<< std::endl;
        free(ptr);
        return 1;
    }

    std::cout << "Values after calloc (all should be 0):" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "cptr[" << i << "] = " << cptr[i] << std::endl;
    }

    // Reallocate memory for 10 integers using realloc
    ptr = (int*)realloc(ptr,10 * sizeof(int));

    if(!ptr){
        std::cerr << "malloc Memory Allocation failed"<< std::endl;
        free(ptr);
        free(cptr);
        return 1;
    }

        // Initialize the new elements
    for (int i = 5; i < 10; ++i) {
        ptr[i] = i * 2;
    }

    // Print the values after realloc
    std::cout << "Values after realloc:" << std::endl;
    for (int i = 0; i < 10; ++i) {
        std::cout << "ptr[" << i << "] = " << ptr[i] << std::endl;
    }

    // Free the allocated memory
    free(ptr);
    free(cptr);

    return 0;
}