#include "../include/mini_syscalls.h"
#include "../include/mini_stdio.h"
#include "../include/mini_stdarg.h"

/*
 * Maximum space needed to print an integer in any base.
 *
 * We set this to log_2(2**64) + 1 + safety margin ~= 80.
 */
#define MAX_INT_BUFF_SIZE  80

/**
 * TODO: need to dispatch to different streams. eg. stdout, stderr...
 * 
 */
static void write_char(void *nchars, int c) {
  int *nchars_printed = (int *)nchars;
  (*nchars_printed)++;
  mini_write(1, &c, 1);
}

/* Write a NUL-terminated string to the given 'write_char' function. */
static void write_string(void *payload, const char *str)
{
    int i;
    for (i = 0; str[i] != 0; i++) {
        write_char(payload, str[i]);
    }
}

/*
 * Write the given unsigned number "n" to the given write_char function.
 *
 * We only support bases up to 16.
 */
static void write_num(void *payload, int base, unsigned long n)
{
    static const char hex[] = "0123456789abcdef";
    char buff[MAX_INT_BUFF_SIZE];
    int k = MAX_INT_BUFF_SIZE - 1;

    /* Special case for "0". */
    if (n == 0) {
        write_string(payload, "0");
        return;
    }

    /* NUL-terminate. */
    buff[k--] = 0;

    /* Generate the number. */
    while (n > 0) {
        buff[k] = hex[n % base];
        n /= base;
        k--;
    }

    /* Print the number. */
    write_string(payload, &buff[k + 1]);
}

/*
 * Print a printf-style string to the given write_char function.
 */
static void mini_vfprintf(void *payload, const char *format, va_list args)
{
    int d, i;
    char c, *s;
    unsigned long p, ul;
    int escape_mode = 0;

    /* Iterate over the format list. */
    for (i = 0; format[i] != 0; i++) {
        /* Handle simple characters. */
        if (!escape_mode && format[i] != '%') {
            write_char(payload, format[i]);
            continue;
        }

        /* Handle the percent escape character. */
        if (format[i] == '%') {
            if (!escape_mode) {
                /* Entering escape mode. */
                escape_mode = 1;
            } else {
                /* Already in escape mode; print a percent. */
                write_char(payload, format[i]);
                escape_mode = 0;
            }
            continue;
        }

        /* Handle the modifier. */
        switch (format[i]) {
        /* Ignore printf modifiers we don't support. */
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '-':
        case '.':
            break;

        /* String. */
        case 's':
            s = va_arg(args, char *);
            write_string(payload, s);
            escape_mode = 0;
            break;

        /* Pointers. */
        case 'p':
            p = va_arg(args, unsigned long);
            write_num(payload, 16, p);
            escape_mode = 0;
            break;

        /* Hex number. */
        case 'x':
            d = va_arg(args, int);
            write_num(payload, 16, d);
            escape_mode = 0;
            break;

        /* Decimal number. */
        case 'd':
        case 'u':
            d = va_arg(args, int);
            write_num(payload, 10, d);
            escape_mode = 0;
            break;

        /* Character. */
        case 'c':
            c = va_arg(args, int);
            write_char(payload, c);
            escape_mode = 0;
            break;

        /* Long number. */
        case 'l':
            switch (format[++i]) {
            case 'u':
                ul = va_arg(args, unsigned long);
                write_num(payload, 10, ul);
                break;

            case 'x':
                ul = va_arg(args, unsigned long);
                write_num(payload, 16, ul);
                break;

            default:
                write_char(payload, '?');
            }
            escape_mode = 0;
            break;

        /* Unknown. */
        default:
            write_char(payload, '?');
            escape_mode = 0;
            break;
        }
    }
}

int mini_printf(const char *restrict fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	mini_vfprintf(&ret, fmt, ap);
	va_end(ap);
	return ret;
}