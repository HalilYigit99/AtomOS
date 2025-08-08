#include <print.h>
#include <stream/OutputStream.h>
#include <stream/VPrintf.h>

static void __wrapper(char c) {
    currentOutputStream->writeChar(c);
}

void kprintf(const char* fmt, ...) {

    if (currentOutputStream == NULL) {
        return; // No output stream available
    }

    va_list args;
    va_start(args, fmt);
    
    // Use the OutputStream to print formatted output
    vprintf(__wrapper, fmt, args);
    
    va_end(args);

}

void printf(const char* fmt, ...) {

    if (currentOutputStream == NULL) {
        return; // No output stream available
    }

    va_list args;
    va_start(args, fmt);
    
    // Use the OutputStream to print formatted output
    vprintf(__wrapper, fmt, args);
    
    va_end(args);

}
