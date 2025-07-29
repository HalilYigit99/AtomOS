#include <stream/OutputStream.h>

extern OutputStream __uart0_output_stream;

OutputStream* currentOutputStream = &__uart0_output_stream;
