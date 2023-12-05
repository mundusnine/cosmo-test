#pragma once
#ifdef _COSMO_SOURCE
#include "libc/c.inc"
#endif
#ifdef DLL_COMPILE
#define FENSTER_API 
#define FENSTER_DLL(name,...) (*name)(__VA_ARGS__) = NULL
#endif
#define FENSTER_HEADER
#include "fenster.h"
typedef struct fenster fenster_t; 