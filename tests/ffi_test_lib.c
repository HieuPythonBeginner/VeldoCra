// Test C library for FFI demonstration
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple math functions
int add_numbers(int a, int b) {
    return a + b;
}

int multiply_numbers(int a, int b) {
    return a * b;
}

// String functions
char* greet_person(const char* name) {
    char* greeting = (char*)malloc(100);
    snprintf(greeting, 100, "Hello, %s! Welcome to VeldoCra FFI!", name);
    return greeting;
}

void free_string(char* str) {
    free(str);
}

// Factorial function
long long factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}