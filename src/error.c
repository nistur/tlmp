#include "tlmp.h"

tlmpReturn g_tlmpError;
const char* g_tlmpErrors[] = 
{
    "Success",
    "Null context",
    "Null device",
    "Device busy",
    "Invalid permissions",
    "I/O error",
    "Memory error"
};
