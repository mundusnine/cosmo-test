#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cosmo.h>

#define _COSMO_SOURCE
#include<libc/dlopen/dlfcn.h>

typedef struct {
    int a;
    int b;
    int* res;
} adder_t;

typedef struct {
    int a;
    int b;
    int* res;
    int c;
} bad_size_adder_t;

int      (*add0)(int a , int b) = NULL;
void     (*add1)(int a,int b, int* res) = NULL;
void*    (*allocate_mem)(size_t size) = NULL;
adder_t* (*create_adder)(int a, int b, int* res) = NULL;
void     (*modify_mem)(adder_t* to_mod) = NULL;
void     (*more_parms)(int a, int b, int* test,int* other_test,int* do_we_have_enough,int* moar,int* moar_moar) = NULL;
#define LOAD_FUN(name) name = cosmo_dlsym(lib,#name)
int main(int argc, char* argv[]){
    void* lib = cosmo_dlopen("./Tests/libdll.so",RTLD_LAZY);
    assert(lib != NULL && "Failed loading dll");

    LOAD_FUN(add0);
    LOAD_FUN(add1);
    LOAD_FUN(allocate_mem);
    LOAD_FUN(create_adder);
    LOAD_FUN(modify_mem);
    LOAD_FUN(more_parms);

    int res = add0(2,2);
    assert(res == 4 && "Function didn't run as intended");

    add1(4,7,&res);
    assert(res == 11 && "Failed passing a variable by pointer");

    void* test2 = allocate_mem(sizeof(adder_t));
    assert(test2 != NULL && 
    "Allocating memory with the malloc in the library doesn't work...");

    adder_t* adder = create_adder(4,7,&res);
    assert(adder != NULL && 
    "Allocating memory with the malloc in the library while passing a pointer from exe doesn't work...");

    adder_t* local_adder = malloc(sizeof(adder_t));
    local_adder->a = 420;
    local_adder->b = 96;
    local_adder->res = &res;
    modify_mem(local_adder);
    assert(res == 111 && "Allocating mem locally and passing it fails...");
    
    int a,b,c,d;
    more_parms(2, 4, &res,&a,&b,&c,&d);
    // *moar_moar = a+b;
    // *moar = 420;
    // *do_we_have_enough = 111-42;
    // *other_test = 42;
    // *test = 99; 
    assert(d == 6 && c == 420 && b == 69 && a == 42 && res == 99 && "Failed this test, more parameters makes this fail.");
}