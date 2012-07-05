/*
 * cpp_stubs.cpp
 *
 *  Created on: 09.06.2012
 *      Author: ryabinin
 */
extern "C"
{
#include <stdlib.h>
}

void *__dso_handle;

void *operator new(size_t size) { return malloc(size); }
void operator delete(void *ptr) { free(ptr); }


