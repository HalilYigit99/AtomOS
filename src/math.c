#include <math.h>
#include <stdint.h>
#include <stdbool.h>

// Basic arithmetic functions
int abs(int x)
{
    return (x < 0) ? -x : x;
}

long labs(long x)
{
    return (x < 0) ? -x : x;
}

long long llabs(long long x)
{
    return (x < 0) ? -x : x;
}

double fabs(double x)
{
    return (x < 0.0) ? -x : x;
}

float fabsf(float x)
{
    return (x < 0.0f) ? -x : x;
}

// Fast square root using bit manipulation (for float)
float fast_sqrtf(float x)
{
    if (x <= 0.0f) return 0.0f;
    
    // Bit-level hacking for fast approximation
    union { float f; uint32_t i; } conv = { .f = x };
    conv.i = 0x5f3759df - (conv.i >> 1); // Magic number
    float y = conv.f;
    
    // Newton-Raphson iteration for better precision
    y = y * (1.5f - (x * 0.5f * y * y));
    y = y * (1.5f - (x * 0.5f * y * y)); // Second iteration for more accuracy
    
    return x * y;
}

double fast_sqrt(double x)
{
    return (double)fast_sqrtf((float)x);
}

// Square root using Newton-Raphson method
double sqrt(double x)
{
    if (x <= 0.0) return 0.0;
    if (x == 1.0) return 1.0;
    
    double guess = x / 2.0;
    double prev_guess = 0.0;
    
    // Newton-Raphson iteration
    while (fabs(guess - prev_guess) > 1e-10)
    {
        prev_guess = guess;
        guess = (guess + x / guess) / 2.0;
    }
    
    return guess;
}

float sqrtf(float x)
{
    return (float)sqrt((double)x);
}

// Power function using exponentiation by squaring for integer exponents
double pow(double base, double exponent)
{
    if (exponent == 0.0) return 1.0;
    if (base == 0.0) return 0.0;
    if (exponent == 1.0) return base;
    
    // For integer exponents, use fast exponentiation
    if (exponent == (int)exponent)
    {
        int exp = (int)exponent;
        bool negative = exp < 0;
        if (negative) exp = -exp;
        
        double result = 1.0;
        double current_base = base;
        
        while (exp > 0)
        {
            if (exp & 1) result *= current_base;
            current_base *= current_base;
            exp >>= 1;
        }
        
        return negative ? 1.0 / result : result;
    }
    
    // For fractional exponents, use exp(exponent * log(base))
    return exp(exponent * log(base));
}

float powf(float base, float exponent)
{
    return (float)pow((double)base, (double)exponent);
}

// Natural logarithm using Taylor series
double log(double x)
{
    if (x <= 0.0) return -HUGE_VAL;  // -INFINITY yerine
    if (x == 1.0) return 0.0;
    
    // Transform x to range [1,2) for better convergence
    int exponent = 0;
    while (x >= 2.0)
    {
        x /= 2.0;
        exponent++;
    }
    while (x < 1.0)
    {
        x *= 2.0;
        exponent--;
    }
    
    // Use Taylor series: ln(1+u) = u - u²/2 + u³/3 - u⁴/4 + ...
    double u = x - 1.0;
    double result = 0.0;
    double term = u;
    
    for (int n = 1; n <= 50; n++)
    {
        result += (n % 2 == 1 ? term : -term) / n;
        term *= u;
        if (fabs(term) < 1e-15) break;
    }
    
    return result + exponent * M_LN2;
}

float logf(float x)
{
    return (float)log((double)x);
}

double log2(double x)
{
    return log(x) / M_LN2;
}

float log2f(float x)
{
    return (float)log2((double)x);
}

double log10(double x)
{
    return log(x) / M_LN10;
}

float log10f(float x)
{
    return (float)log10((double)x);
}

// Exponential function using Taylor series
double exp(double x)
{
    if (x == 0.0) return 1.0;
    
    // Handle large values to prevent overflow
    if (x > 700.0) return HUGE_VAL;  // INFINITY yerine
    if (x < -700.0) return 0.0;
    
    // Use Taylor series: e^x = 1 + x + x²/2! + x³/3! + ...
    double result = 1.0;
    double term = 1.0;
    
    for (int n = 1; n <= 50; n++)
    {
        term *= x / n;
        result += term;
        if (fabs(term) < 1e-15) break;
    }
    
    return result;
}

float expf(float x)
{
    return (float)exp((double)x);
}

double exp2(double x)
{
    return pow(2.0, x);
}

float exp2f(float x)
{
    return (float)exp2((double)x);
}

// Trigonometric functions using Taylor series
double sin(double x)
{
    // Reduce x to range [-π, π]
    while (x > M_PI) x -= 2.0 * M_PI;
    while (x < -M_PI) x += 2.0 * M_PI;
    
    // Taylor series: sin(x) = x - x³/3! + x⁵/5! - x⁷/7! + ...
    double result = 0.0;
    double term = x;
    double x_squared = x * x;
    
    for (int n = 1; n <= 25; n += 2)
    {
        result += (((n-1)/2) % 2 == 0 ? term : -term) / factorial(n);
        term *= x_squared;
        if (fabs(term) < 1e-15) break;
    }
    
    return result;
}

double fast_sin(double x)
{
    // Fast approximation using polynomial
    while (x > M_PI) x -= 2.0 * M_PI;
    while (x < -M_PI) x += 2.0 * M_PI;
    
    if (x < 0)
        return -fast_sin(-x);
    
    if (x > M_PI_2)
        return fast_cos(x - M_PI_2);
    
    double x2 = x * x;
    return x * (1.0 - x2 * (1.0/6.0 - x2 * (1.0/120.0 - x2/5040.0)));
}

float sinf(float x)
{
    return (float)sin((double)x);
}

double cos(double x)
{
    return sin(x + M_PI_2);
}

double fast_cos(double x)
{
    return fast_sin(x + M_PI_2);
}

float cosf(float x)
{
    return (float)cos((double)x);
}

double tan(double x)
{
    double cos_x = cos(x);
    if (fabs(cos_x) < 1e-15) return HUGE_VAL;  // INFINITY yerine
    return sin(x) / cos_x;
}

float tanf(float x)
{
    return (float)tan((double)x);
}

// Rounding functions
double floor(double x)
{
    if (x >= 0.0)
        return (double)(long long)x;
    else
    {
        long long i = (long long)x;
        return (x == (double)i) ? x : (double)(i - 1);
    }
}

float floorf(float x)
{
    return (float)floor((double)x);
}

double ceil(double x)
{
    if (x <= 0.0)
        return (double)(long long)x;
    else
    {
        long long i = (long long)x;
        return (x == (double)i) ? x : (double)(i + 1);
    }
}

float ceilf(float x)
{
    return (float)ceil((double)x);
}

double round(double x)
{
    return floor(x + 0.5);
}

float roundf(float x)
{
    return (float)round((double)x);
}

double trunc(double x)
{
    return (double)(long long)x;
}

float truncf(float x)
{
    return (float)trunc((double)x);
}

double fmod(double x, double y)
{
    if (y == 0.0) return 0.0;
    return x - trunc(x / y) * y;
}

float fmodf(float x, float y)
{
    return (float)fmod((double)x, (double)y);
}

// Comparison functions
double fmax(double x, double y)
{
    return (x > y) ? x : y;
}

float fmaxf(float x, float y)
{
    return (x > y) ? x : y;
}

double fmin(double x, double y)
{
    return (x < y) ? x : y;
}

float fminf(float x, float y)
{
    return (x < y) ? x : y;
}

// Utility functions
double copysign(double x, double y)
{
    return (y >= 0.0) ? fabs(x) : -fabs(x);
}

float copysignf(float x, float y)
{
    return (y >= 0.0f) ? fabsf(x) : -fabsf(x);
}

// Integer math utilities
int gcd(int a, int b)
{
    a = abs(a);
    b = abs(b);
    while (b != 0)
    {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

int lcm(int a, int b)
{
    return abs(a * b) / gcd(a, b);
}

int ipow(int base, int exp)
{
    if (exp < 0) return 0;
    int result = 1;
    while (exp > 0)
    {
        if (exp & 1) result *= base;
        base *= base;
        exp >>= 1;
    }
    return result;
}

uint32_t isqrt(uint32_t n)
{
    if (n == 0) return 0;
    
    uint32_t x = n;
    uint32_t y = (x + 1) / 2;
    
    while (y < x)
    {
        x = y;
        y = (x + n / x) / 2;
    }
    
    return x;
}

// Helper function for Taylor series
double factorial(int n)
{
    if (n <= 1) return 1.0;
    double result = 1.0;
    for (int i = 2; i <= n; i++)
        result *= i;
    return result;
}

// Classification functions (basic implementations)
int isfinite(double x)
{
    return x == x && x != HUGE_VAL && x != -HUGE_VAL;
}

int isinf(double x)
{
    return x == HUGE_VAL || x == -HUGE_VAL;
}

int isnan(double x)
{
    return x != x;
}

int isnormal(double x)
{
    return isfinite(x) && x != 0.0;
}

int signbit(double x)
{
    return x < 0.0;
}
