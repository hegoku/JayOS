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

// int vsprintf(char *buf, const char *format, va_list args)
// {
// 	char* p;

// 	va_list	p_next_arg = args;
// 	int	m;

// 	char cs;
// 	int	align_nr;

// 	for (p=buf;*format;format++) {
// 		if (*format != '%') {
// 			*p++ = *format;
// 			continue;
// 		} else {		/* a format string begins */
// 			align_nr = 0;
// 		}

// 		format++;

// 		if (*format == '%') {
// 			*p++ = *format;
// 			continue;
// 		} else if (*format == '0') {
// 			cs = '0';
// 			format++;
// 		} else {
// 			cs = ' ';
// 		}
// 		while (((unsigned char)(*format) >= '0') && ((unsigned char)(*format) <= '9')) {
// 			align_nr *= 10;
// 			align_nr += *format - '0';
// 			format++;
// 		}

// 		char inner_buf[STR_DEFAULT_LEN];
//         char *q = inner_buf;
// 		memset(q, 0, STR_DEFAULT_LEN);

// 		switch (*format)
//         {
//         case 'c':
// 			*q++ = *((char*)p_next_arg);
// 			p_next_arg += 4;
// 			break;
// 		case 'x':
// 			m = *((int*)p_next_arg);
// 			i2a(m, 16, &q);
// 			p_next_arg += 4;
// 			break;
// 		case 'd':
//         case 'i':
// 			m = *((int*)p_next_arg);
// 			if (m < 0) {
// 				m = m * (-1);
// 				*q++ = '-';
// 			}
// 			i2a(m, 10, &q);
// 			p_next_arg += 4;
// 			break;
// 		case 's':
// 			strcpy(q, (*((char**)p_next_arg)));
// 			q += strlen(*((char**)p_next_arg));
// 			p_next_arg += 4;
// 			break;
// 		default:
// 			break;
// 		}

// 		int k;
// 		for (k = 0; k < ((align_nr > strlen(inner_buf)) ? (align_nr - strlen(inner_buf)) : 0); k++) {
// 			*p++ = cs;
// 		}
// 		q = inner_buf;
// 		while (*q) {
// 			*p++ = *q++;
// 		}
// 	}

// 	*p = 0;

//     return (p - buf);
// }

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
    write(STDIN_FILENO, buf, i);
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


/* we use this so that we can do without the ctype library */
#define is_digit(c)	((c) >= '0' && (c) <= '9')

static int skip_atoi(const char **s)
{
	int i=0;

	while (is_digit(**s))
		i = i*10 + *((*s)++) - '0';
	return i;
}

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define SMALL	64		/* use 'abcdef' instead of 'ABCDEF' */

#define do_div(n,base) ({ \
int __res; \
__asm__("divl %4":"=a" (n),"=d" (__res):"0" (n),"1" (0),"r" (base)); \
__res; })

static char * number(char * str, int num, int base, int size, int precision
	,int type)
{
	char c,sign,tmp[36];
	const char *digits="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i;

	if (type&SMALL) digits="0123456789abcdefghijklmnopqrstuvwxyz";
	if (type&LEFT) type &= ~ZEROPAD;
	if (base<2 || base>36)
		return 0;
	c = (type & ZEROPAD) ? '0' : ' ' ;
	if (type&SIGN && num<0) {
		sign='-';
		num = -num;
	} else
		sign=(type&PLUS) ? '+' : ((type&SPACE) ? ' ' : 0);
	if (sign) size--;
	if (type&SPECIAL)
		if (base==16) size -= 2;
		else if (base==8) size--;
	i=0;
	if (num==0)
		tmp[i++]='0';
	else while (num!=0)
		tmp[i++]=digits[do_div(num,base)];
	if (i>precision) precision=i;
	size -= precision;
	if (!(type&(ZEROPAD+LEFT)))
		while(size-->0)
			*str++ = ' ';
	if (sign)
		*str++ = sign;
	if (type&SPECIAL)
		if (base==8)
			*str++ = '0';
		else if (base==16) {
			*str++ = '0';
			*str++ = digits[33];
		}
	if (!(type&LEFT))
		while(size-->0)
			*str++ = c;
	while(i<precision--)
		*str++ = '0';
	while(i-->0)
		*str++ = tmp[i];
	while(size-->0)
		*str++ = ' ';
	return str;
}


int vsprintf(char *buf, const char *fmt, va_list args)
{
	int len;
	int i;
	char * str;
	char *s;
	int *ip;

	int flags;		/* flags to number() */

	int field_width;	/* width of output field */
	int precision;		/* min. # of digits for integers; max
				   number of chars for from string */
	int qualifier;		/* 'h', 'l', or 'L' for integer fields */

	for (str=buf ; *fmt ; ++fmt) {
		if (*fmt != '%') {
			*str++ = *fmt;
			continue;
		}
			
		/* process flags */
		flags = 0;
		repeat:
			++fmt;		/* this also skips first '%' */
			switch (*fmt) {
				case '-': flags |= LEFT; goto repeat;
				case '+': flags |= PLUS; goto repeat;
				case ' ': flags |= SPACE; goto repeat;
				case '#': flags |= SPECIAL; goto repeat;
				case '0': flags |= ZEROPAD; goto repeat;
				}
		
		/* get field width */
		field_width = -1;
		if (is_digit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*') {
			/* it's the next argument */
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* get the precision */
		precision = -1;
		if (*fmt == '.') {
			++fmt;	
			if (is_digit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == '*') {
				/* it's the next argument */
				precision = va_arg(args, int);
			}
			if (precision < 0)
				precision = 0;
		}

		/* get the conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
			qualifier = *fmt;
			++fmt;
		}

		switch (*fmt) {
		case 'c':
			if (!(flags & LEFT))
				while (--field_width > 0)
					*str++ = ' ';
			*str++ = (unsigned char) va_arg(args, int);
			while (--field_width > 0)
				*str++ = ' ';
			break;

		case 's':
			s = va_arg(args, char *);
			len = strlen(s);
			if (precision < 0)
				precision = len;
			else if (len > precision)
				len = precision;

			if (!(flags & LEFT))
				while (len < field_width--)
					*str++ = ' ';
			for (i = 0; i < len; ++i)
				*str++ = *s++;
			while (len < field_width--)
				*str++ = ' ';
			break;

		case 'o':
			str = number(str, va_arg(args, unsigned long), 8,
				field_width, precision, flags);
			break;

		case 'p':
			if (field_width == -1) {
				field_width = 8;
				flags |= ZEROPAD;
			}
			str = number(str,
				(unsigned long) va_arg(args, void *), 16,
				field_width, precision, flags);
			break;

		case 'x':
			flags |= SMALL;
		case 'X':
			str = number(str, va_arg(args, unsigned long), 16,
				field_width, precision, flags);
			break;

		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			str = number(str, va_arg(args, unsigned long), 10,
				field_width, precision, flags);
			break;

		case 'n':
			ip = va_arg(args, int *);
			*ip = (str - buf);
			break;

		default:
			if (*fmt != '%')
				*str++ = '%';
			if (*fmt)
				*str++ = *fmt;
			else
				--fmt;
			break;
		}
	}
	*str = '\0';
	return str-buf;
}