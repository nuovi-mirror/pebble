#include "nums.h"
#include "main.h"

/* string -> num */

long str2l
(const char *ptr)
{
	if (ptr == NULL) return 0;

	unsigned int neg = FALSE;
	unsigned int c = 0;
	long res = 0;

	if (ptr[c] == '-') {
		neg = TRUE;
		c++;
	}

	while (ptr[c] == '0') c++;

	while (ptr[c] != '\0')
	{
		if (ptr[c] < 48 || ptr[c] > 57) return 0;
		unsigned long temp = ptr[c] - '0';
		if (res > (ULONG_MAX - temp) / 10) return 0;
		res = res * 10 + temp;
		c++;
	}

	if (neg == TRUE) res = -(res);

	return res;
}

unsigned long str2ul
(const char *ptr)
{
	if (ptr == NULL) return 0;

	unsigned int c = 0;
	unsigned long res = 0;

	while (ptr[c] == '0') c++;

	while (ptr[c] != '\0')
	{
		if (ptr[c] < 48 || ptr[c] > 57) return 0;
		unsigned long temp = ptr[c] - '0';
		if (res > (ULONG_MAX - temp) / 10) return 0;
		res = res * 10 + temp;
		c++;
	}

	return res;
}

/* num -> string */

int l2str 
(char *buff, unsigned long buffsize, long n)
{
	long i = 0;
	long neg = (n < 0);
	if (buffsize == 0) return 0;

	do
	{
		if (i >= buffsize - 1)
		{
			buff[i++] = '\0';
			return i;
		}

		long d = n % 10;
		if (d < 0) d = -d;
		buff[i++] = (char)(d + '0');
		n /= 10;
	} while (n != 0);

	if (neg) buff[i++] = '-';
	buff[i] = '\0';

	for (long j = 0; j < i / 2; j++)
	{
		char tmp = buff[j];
		buff[j] = buff[i - j - 1];
		buff[i - j - 1] = tmp;
	}

	return i;
}

int ul2str 
(char *buff, unsigned long buffsize, unsigned long n)
{
	unsigned long i = 0;
	if (buffsize == 0) return 0;

	do
	{
		if (i >= buffsize - 1)
		{
			buff[i++] = '\0';
			return i;
		}

		unsigned long d = n % 10;
		buff[i++] = (char)(d + '0');
		n /= 10;
	} while (n != 0);

	buff[i] = '\0';

	for (unsigned long j = 0; j < i / 2; j++)
	{
		char tmp = buff[j];
		buff[j] = buff[i - j - 1];
		buff[i - j - 1] = tmp;
	}

	return i;
}

/* same thing but VM types */

/* string -> num */

sword strtosword
(const char *ptr)
{
	if (ptr == NULL) return 0;

	unsigned int neg = FALSE;
	unsigned int c = 0;
	sword res = 0;

	if (ptr[c] == '-') {
		neg = TRUE;
		c++;
	}

	while (ptr[c] == '0') c++;

	while (ptr[c] != '\0')
	{
		if (ptr[c] < 48 || ptr[c] > 57) return 0;
		unsigned long temp = ptr[c] - '0';
		if (res > (SWORD_MAX - temp) / 10) return 0;
		res = res * 10 + temp;
		c++;
	}

	if (neg == TRUE) res = -(res);

	return res;
}

word strtoword
(const char *ptr)
{
	if (ptr == NULL) return 0;

	unsigned int c = 0;
	word res = 0;

	while (ptr[c] == '0') c++;

	while (ptr[c] != '\0')
	{
		if (ptr[c] < 48 || ptr[c] > 57) return 0;
		unsigned long temp = ptr[c] - '0';
		if (res > (WORD_MAX - temp) / 10) return 0;
		res = res * 10 + temp;
		c++;
	}

	return res;
}

/* num -> string */

int swordtostr 
(char *buff, unsigned long buffsize, sword n)
{
	sword i = 0;
	sword neg = (n < 0);
	if (buffsize == 0) return 0;

	do
	{
		if (i >= buffsize - 1)
		{
			buff[i++] = '\0';
			return i;
		}

		sword d = n % 10;
		if (d < 0) d = -d;
		buff[i++] = (char)(d + '0');
		n /= 10;
	} while (n != 0);

	if (neg) buff[i++] = '-';
	buff[i] = '\0';

	for (sword j = 0; j < i / 2; j++)
	{
		char tmp = buff[j];
		buff[j] = buff[i - j - 1];
		buff[i - j - 1] = tmp;
	}

	return i;
}

int wordtostr
(char *buff, unsigned long buffsize, word n)
{
	word i = 0;
	if (buffsize == 0) return 0;

	do
	{
		if (i >= buffsize - 1)
		{
			buff[i++] = '\0';
			return i;
		}

		word d = n % 10;
		buff[i++] = (char)(d + '0');
		n /= 10;
	} while (n != 0);

	buff[i] = '\0';

	for (word j = 0; j < i / 2; j++)
	{
		char tmp = buff[j];
		buff[j] = buff[i - j - 1];
		buff[i - j - 1] = tmp;
	}

	return i;
}
