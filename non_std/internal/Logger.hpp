#pragma once

/* Uncomment to print library internals */
/* Or define this macro in single file */
// #define LOG_HEAVLY

#ifdef LOG_HEAVLY
#include <iostream>
#define LOG(tail) std::cout << tail
#else
#define LOG(tail) do { } while(0);
#endif
