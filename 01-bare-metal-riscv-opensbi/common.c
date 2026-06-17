#include "common.h"

void putchar(char ch);

void printf(const char *fmt, ...) {
    va_list vargs;
    va_start(vargs, fmt);

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;          // Skip '%'
            switch (*fmt) { // Считываем следующий символ
            case '\0':      // '%' в конце строки формата.
                putchar('%');
                goto end;
            case '%': // Выводим '%'
                putchar('%');
                break;
            case 's': { // Выводим NULL-терминированную строку.
                const char *s = va_arg(vargs, const char *);
                while (*s) {
                    putchar(*s);
                    s++;
                }
                break;
            }
            case 'd': { // Выводим целое число в десятичном формате.
                int value = va_arg(vargs, int);
                if (value < 0) {
                    putchar('-');
                    value = -value;
                }

                int divisor = 1;
                while (value / divisor > 9)
                    divisor *= 10;

                while (divisor > 0) {
                    putchar('0' + value / divisor);
                    value %= divisor;
                    divisor /= 10;
                }

                break;
            }
            case 'x': { // Выводим целое число в шестнадцатеричном формате.
                int value = va_arg(vargs, int);
                for (int i = 7; i >= 0; i--) {
                    int nibble = (value >> (i * 4)) & 0xf;
                    putchar("0123456789abcdef"[nibble]);
                }
            }
            }
        } else {
            putchar(*fmt);
        }

        fmt++;
    }

end:
    va_end(vargs);
}
