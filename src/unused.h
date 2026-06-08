#ifndef UNUSED_H
#define UNUSED_H

#define UNUSED(x)       (void)x

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
        #define UNUSED_VARIABLE [[maybe_unused]]
        #define UNUSED_FUNCTION [[maybe_unused]]
#elif defined(__GNUC__)
        #define UNUSED_VARIABLE __attribute__((unused))
        #define UNUSED_FUNCTION __attribute__((unused))
#else
        #define UNUSED_VARIABLE
        #define UNUSED_FUNCTION
#endif

#endif // UNUSED_H
