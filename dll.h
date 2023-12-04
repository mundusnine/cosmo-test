#pragma once
#include <stdio.h>

typedef struct {
    int a;
    int b;
    int* res;
} adder_t;

int add0(int a , int b);
void add1(int a,int b, int* res);
void* allocate_mem(size_t size);
adder_t* create_adder(int a, int b, int* res);
void modify_mem(adder_t* to_mod);
void more_parms(int a, int b, int* test,int* other_test,int* do_we_have_enough,int* moar,int* moar_moar);