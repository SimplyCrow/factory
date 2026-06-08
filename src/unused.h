#ifndef UNUSED_H
#define UNUSED_H

#define UNUSED(x)       (void)x
#define UNUSED_FUNCTION __attribute__((unused))
#define UNUSED_VARIABLE [[maybe_unused]]

#endif // UNUSED_H
