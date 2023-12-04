#include "dll.h"
#include <stdlib.h>
#include <assert.h>

int add0(int a , int b){
    return a+b;
}
void add1(int a,int b, int* res){
    *res = a+b;
}
void* allocate_mem(size_t size){
    return malloc(size);
}
adder_t* create_adder(int a, int b, int* res){
    adder_t* adder = malloc(sizeof(adder_t));
    assert(adder != NULL && "Failed to allocate data.");
    adder->a = a;
    adder->b = b;
    adder->res = res;
    return adder;
}
void modify_mem(adder_t* to_mod){
    to_mod->a = 42;
    to_mod->b = 69;
    *to_mod->res = to_mod->a + to_mod->b; 
}
void more_parms(int a, int b, int* test,int* other_test,int* do_we_have_enough,int* moar,int* moar_moar){
    *moar_moar = a+b;
    *moar = 420;
    *do_we_have_enough = 111-42;
    *other_test = 42;
    *test = 99; 
}