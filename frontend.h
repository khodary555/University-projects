#ifndef FRONTEND_H_INCLUDED
#define FRONTEND_H_INCLUDED

#include "backend.h"

#define COLOR_GREEN "\033[1;32m"
#define COLOR_RED "\033[1;31m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_RESET "\033[0m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[1;37m"

void clearScreen()
{
#ifdef _WIN32
	system("cls");
#else
	system("clear");
#endif
}

void pauseScreen()
{
	printf("\nPress ENTER to continue...\n");
	fflush(stdout);

	getchar();
}

void trimWhitespace(char *str)
{
	while (isspace((unsigned char)*str))
		str++;
	char *end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char)*end))
		end--;
	*(end + 1) = '\0';
}
void trimNewline(char *str)
{
	str[strcspn(str, "\n")] = '\0';
}
#endif // FRONTEND_H_INCLUDED