#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define STR_DEFAULT_LEN 1024

static char* i2a(int val, int base, char ** ps)
{
	int m = val % base;
	int q = val / base;
	if (q) {
		i2a(q, base, ps);
	}
	*(*ps)++ = (m < 10) ? (m + '0') : (m - 10 + 'A');

	return *ps;
}

int vsprintf(char *buf, const char *format, va_list args)
{
	char* p;

	va_list	p_next_arg = args;
	int	m;

	char cs;
	int	align_nr;

	for (p=buf;*format;format++) {
		if (*format != '%') {
			*p++ = *format;
			continue;
		} else {		/* a format string begins */
			align_nr = 0;
		}

		format++;

		if (*format == '%') {
			*p++ = *format;
			continue;
		} else if (*format == '0') {
			cs = '0';
			format++;
		} else {
			cs = ' ';
		}
		while (((unsigned char)(*format) >= '0') && ((unsigned char)(*format) <= '9')) {
			align_nr *= 10;
			align_nr += *format - '0';
			format++;
		}

		char inner_buf[STR_DEFAULT_LEN];
        char * q = inner_buf;

        switch (*format)
        {
        case 'c':
			*q++ = *((char*)p_next_arg);
			p_next_arg += 4;
			break;
		case 'x':
			m = *((int*)p_next_arg);
			i2a(m, 16, &q);
			p_next_arg += 4;
			break;
		case 'd':
        case 'i':
			m = *((int*)p_next_arg);
			if (m < 0) {
				m = m * (-1);
				*q++ = '-';
			}
			i2a(m, 10, &q);
			p_next_arg += 4;
			break;
		case 's':
			strcpy(q, (*((char**)p_next_arg)));
			q += strlen(*((char**)p_next_arg));
			p_next_arg += 4;
			break;
		default:
			break;
		}

		int k;
		for (k = 0; k < ((align_nr > strlen(inner_buf)) ? (align_nr - strlen(inner_buf)) : 0); k++) {
			*p++ = cs;
		}
		q = inner_buf;
		while (*q) {
			*p++ = *q++;
		}
	}

	*p = 0;

    return (p - buf);
}

int sprintf(char *buf, const char *format, ...)
{
	va_list arg = (va_list)((char*)(&format) + 4);        /* 4 是参数 format 所占堆栈中的大小 */
	return vsprintf(buf, format, arg);
}

int printf(const char *format, ...)
{
    int i;
    char buf[STR_DEFAULT_LEN];
    va_list arg = (va_list)((char*)(&format)+4);//4为format所占堆栈中大小
    i = vsprintf(buf, format, arg);
    write(1, buf, i);
    return i;
}

int sscanf(const char *str, const char *format, ...)
{
    return 0;
}

int scanf(const char *format, ...)
{
    int count;
    char buf[STR_DEFAULT_LEN];
    va_list arg = (va_list)((char*)(&format) + 4);
    count = vsscanf (buf, format, arg);
    return (count);
}

static int vsscanf (const char *buf, const char *s, va_list ap)
{
    int             count, noassign, width, base, lflag;
    const char     *tc;
    char           *t, tmp[STR_DEFAULT_LEN];

    count = noassign = width = lflag = 0;
    // while (*s && *buf) {
	// while (isspace (*s))
	//     s++;
	// if (*s == '%') {
	//     s++;
	//     for (; *s; s++) {
	// 	if (strchr ("dibouxcsefg%", *s))
	// 	    break;
	// 	if (*s == '*')
	// 	    noassign = 1;
	// 	else if (*s == 'l' || *s == 'L')
	// 	    lflag = 1;
	// 	else if (*s >= '1' && *s <= '9') {
	// 	    for (tc = s; isdigit (*s); s++);
	// 	    strncpy (tmp, tc, s - tc);
	// 	    tmp[s - tc] = '\0';
	// 	    atob (&width, tmp, 10);
	// 	    s--;
	// 	}
	//     }
	//     if (*s == 's') {
	// 	while (isspace (*buf))
	// 	    buf++;
	// 	if (!width)
	// 	    width = strcspn (buf, ISSPACE);
	// 	if (!noassign) {
	// 	    strncpy (t = va_arg (ap, char *), buf, width);
	// 	    t[width] = '\0';
	// 	}
	// 	buf += width;
	//     } else if (*s == 'c') {
	// 	if (!width)
	// 	    width = 1;
	// 	if (!noassign) {
	// 	    strncpy (t = va_arg (ap, char *), buf, width);
	// 	    t[width] = '\0';
	// 	}
	// 	buf += width;
	//     } else if (strchr ("dobxu", *s)) {
	// 	while (isspace (*buf))
	// 	    buf++;
	// 	if (*s == 'd' || *s == 'u')
	// 	    base = 10;
	// 	else if (*s == 'x')
	// 	    base = 16;
	// 	else if (*s == 'o')
	// 	    base = 8;
	// 	else if (*s == 'b')
	// 	    base = 2;
	// 	if (!width) {
	// 	    if (isspace (*(s + 1)) || *(s + 1) == 0)
	// 		width = strcspn (buf, ISSPACE);
	// 	    else
	// 		width = strchr (buf, *(s + 1)) - buf;
	// 	}
	// 	strncpy (tmp, buf, width);
	// 	tmp[width] = '\0';
	// 	buf += width;
	// 	if (!noassign)
	// 	    atob (va_arg (ap, u_int32_t *), tmp, base);
	//     }
	//     if (!noassign)
	// 	count++;
	//     width = noassign = lflag = 0;
	//     s++;
	// } else {
	//     while (isspace (*buf))
	// 	buf++;
	//     if (*s != *buf)
	// 	break;
	//     else
	// 	s++, buf++;
	// }
    // }
    return (count);
}