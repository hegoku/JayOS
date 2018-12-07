#include <string.h>

char *strchr(const char *str, int c)
{
	for (; *str != (char)c; ++str)
		if (*str == '\0')
			return NULL;
	return (char *)str;
}

char *strncpy(char *dest, const char *src, size_t n)
{
	char *tmp = dest;

	while (n) {
		if ((*tmp = *src) != 0)
			src++;
		tmp++;
		n--;
	}
	return dest;
}

size_t strcspn(const char *str1, const char *str2)
{
	const char *p;
	const char *r;
	size_t count = 0;

	for (p = str1; *p != '\0'; ++p) {
		for (r = str2; *r != '\0'; ++r) {
			if (*p == *r)
				return count;
		}
		++count;
	}
	return count;
}

size_t strcmp(const char * str1, const char *str2)
{
	if ((str1 == 0) || (str2 == 0)) { /* for robustness */
		return (str1 - str2);
	}

	const char * p1 = str1;
	const char * p2 = str2;

	for (; *p1 && *p2; p1++,p2++) {
		if (*p1 != *p2) {
			break;
		}
	}

	return (*p1 - *p2);
}

void *memcpy(void *dest, const void *src, size_t n)
{
	char *tmp = dest;
	const char *s = src;

	while (n--)
		*tmp++ = *s++;
	return dest;
}

int memcmp(const void *str1, const void *str2, size_t n)
{
	const unsigned char *su1, *su2;
	int res = 0;

	for (su1 = str1, su2 = str2; 0 < n; ++su1, ++su2, n--)
		if ((res = *su1 - *su2) != 0)
			break;
	return res;
}

// void *memset(void *str, int c, size_t n)
// {
// 	char *xs = str;

// 	while (n--)
// 		*xs++ = c;
// 	return str;
// }