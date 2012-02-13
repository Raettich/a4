#ifndef __A4_DEBUG_H
#define __A4_DEBUG_H

#include <iostream>

#include <a4/string.h>
#include <a4/types.h>

#define A4_LOG_PREFIX(severity) \
    "[" severity "] " 

#define FATAL(...) \
    do { \
        throw a4::Fatal(__VA_ARGS__); \
    } while(0)

#define ERROR(...)   std::cerr << A4_LOG_PREFIX("ERROR") << str_cat(  __VA_ARGS__  ) << std::endl;
#define WARNING(...) std::cerr << A4_LOG_PREFIX("WARNING") << str_cat(  __VA_ARGS__  ) << std::endl;
#define INFO(...)    std::cerr << A4_LOG_PREFIX("INFO") << str_cat(  __VA_ARGS__  ) << std::endl;
#define VERBOSE(...) std::cerr << A4_LOG_PREFIX("VERBOSE") << str_cat(  __VA_ARGS__  ) << std::endl;

#ifdef NDEBUG
    #define DEBUG(...)
    #define DEBUG_ASSERT(A, ...)
#else
    #define DEBUG(...)   std::cerr << A4_LOG_PREFIX("") << str_cat(  __VA_ARGS__  ) << std::endl;
    #define DEBUG_ASSERT(A, ...) do { if(!(A)) { DEBUG( __VA_ARGS__ ); } } while (0)
#endif

#define ERROR_ASSERT(A, ...) do { if(!(A)) { ERROR( ##__VA_ARGS__ ); } } while (0)
#define WARNING_ASSERT(A, ...) do { if(!(A)) { WARNING( ##__VA_ARGS__ ); } } while (0)
#define INFO_ASSERT(A, ...) do { if(!(A)) { INFO( ##__VA_ARGS__ ); } } while (0)
#define VERBOSE_ASSERT(A, ...) do { if(!(A)) { VERBOSE( ##__VA_ARGS__ ); } } while (0)

#endif // __A4_DEBUG_H

