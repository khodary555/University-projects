#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "frontend.h"
#define DAILY_WITHDRAW_LIMIT 10000.0
#define DAILY_TRANSFER_LIMIT 50000.0

int validEmail(const char *email)
{
	if (email == NULL || email[0] == '\0' || strchr(email, ' ') != NULL)
	{
		return 0;
	}

	char *at_symbol = strchr(email, '@');
	if (at_symbol == NULL || strrchr(email, '@') != at_symbol)
	{
		return 0;
	}
	if (at_symbol == email)
	{
		return 0;
	}
	char *domain_part = at_symbol + 1;
	if (domain_part[0] == '\0')
	{
		return 0;
	}
	char *last_dot = strrchr(domain_part, '.');
	if (last_dot == NULL || last_dot == domain_part)
	{
		return 0;
	}
	if (strstr(email, "..") != NULL)
	{
		return 0;
	}
	if (strlen(last_dot + 1) < 2)
	{
		return 0;
	}
	return 1;
}

int getIntInRange(const char *prompt, int min, int max)
{
	char buf[100];
	int value;
	char extra;

	while (1)
	{
		printf("%s", prompt);

		if (!fgets(buf, sizeof(buf), stdin))
			continue; // read error, try again
		int count = sscanf(buf, "%d %c", &value, &extra);

		if (count != 1)
		{
			printf(COLOR_RED "Invalid input. Please enter a number.\n" COLOR_RESET);
			continue;
		}

		if (value < min || value > max)
		{
			printf(COLOR_RED "Invalid choice. Must be between %d and %d.\n" COLOR_RESET, min, max);
			continue;
		}
		return value;
	}
}

int confirmChanges(void)
{
	printf("\n" COLOR_CYAN "Confirm changes?\n" COLOR_RESET);
	printf("1. Confirm\n");
	printf("2. Discard\n");

	int choice = getIntInRange(COLOR_YELLOW "Enter choice (1-2): " COLOR_RESET, 1, 2);
	return choice;
}

int MENU(int loggedIn);

typedef struct
{
	int month;
	int year;
} Date;

typedef struct
{
	char accountNumber[20];
	char name[50];
	char email[50];
	float balance;
	char mobileNumber[15];
	Date dateOpened;
	char status[10];
	float dailyWithdrawn;
	float dailyRecieved;
	float dailyDeposited;
	float dailyTransferred;
} Account;

typedef struct
{
	Account accounts[1000];
	int count;
} AccountList;

void clearStdin();
AccountList LOAD();
void createfiles(AccountList *List);

void appendTransaction(AccountList *List, char *accNumber, char *type, float amount, char *details);

void toTitleCase(char *str)
{
	if (str == NULL || str[0] == '\0')
	{
		return;
	}

	int isNewWord = 1;

	for (int i = 0; str[i] != '\0'; i++)
	{
		if (isspace((unsigned char)str[i]))
		{
			isNewWord = 1;
		}
		else if (isNewWord && isalpha((unsigned char)str[i]))
		{
			str[i] = toupper((unsigned char)str[i]);
			isNewWord = 0;
		}
		else
		{
			str[i] = tolower((unsigned char)str[i]);
		}
	}
}

int compareDates(Date fileDate, int targetMonth, int targetYear)
{
	return (fileDate.month == targetMonth && fileDate.year == targetYear);
}

int parseTargetDate(char *input, int *outMonth, int *outYear)
{

	if (sscanf(input, "%d-%d", outMonth, outYear) == 2)
	{
		if (*outMonth >= 1 && *outMonth <= 12)
			return 1;
	}
	return 0;
}

void SAVE(AccountList *List)
{
	FILE *file = fopen("accounts.txt", "w");
	if (file == NULL)
	{
		printf("Error opening accounts.txt for saving");
		printf("Failed to save account data.\n");
		return;
	}
	int i;
	for (i = 0; i < List->count; i++)
	{
		Account *acc = &List->accounts[i];
		trimWhitespace(acc->accountNumber);
		trimWhitespace(acc->name);
		trimWhitespace(acc->email);
		trimWhitespace(acc->mobileNumber);
		trimWhitespace(acc->status);
		fprintf(
			file,
			"%s,%s,%s,%.2f,%s,%d-%d,%s\n",
			acc->accountNumber,
			acc->name,
			acc->email,
			acc->balance,
			acc->mobileNumber,
			acc->dateOpened.month,
			acc->dateOpened.year,
			acc->status);
	}

	fclose(file);
	printf("Successfully saved %d accounts to file.\n", List->count);
}

void QUIT()
{
	printf(COLOR_RED "Exiting program.\n" COLOR_RESET);
	exit(0);
}
void resetDailyLimits(AccountList *List)
{
	time_t t = time(NULL);
	struct tm *tm_info = localtime(&t);

	int currentMonth = tm_info->tm_mon + 1;
	int currentYear = tm_info->tm_year + 1900;

	static int lastResetMonth = -1;
	static int lastResetYear = -1;
	if (lastResetMonth == currentMonth && lastResetYear == currentYear)
		return;

	for (int i = 0; i < List->count; i++)
	{
		List->accounts[i].dailyWithdrawn = 0.0;
		List->accounts[i].dailyRecieved = 0.0;
		List->accounts[i].dailyDeposited = 0.0;
		List->accounts[i].dailyTransferred = 0.0;
	}

	lastResetMonth = currentMonth;
	lastResetYear = currentYear;
}

int isActive(const char *status)
{
	char temp[20];
	strcpy(temp, status);
	trimWhitespace(temp);

	return (strcmp(temp, "active") == 0);
}

int isValid(char *accountNumber)
{
	int valid = 0;
	if (strlen(accountNumber) != 10)
	{
		printf("Invalid account number. Must be 10 digits.\n");
		return valid;
	}
	int i = 0;
	for (i = 0; i < 10; i++)
	{
		if (accountNumber[i] < '0' || accountNumber[i] > '9')
		{
			valid = 0;
			printf("Invalid account number. Digits only.\n");
			return valid;
		}
	}
	return 1;
}

void clearStdin()
{
	int c;
	while ((c = getchar()) != '\n' && c != EOF)
		;
}

int findAccount(AccountList *list, char NumberToModify[])
{
	int i;
	for (i = 0; i < list->count; i++)
	{
		if (strcmp(list->accounts[i].accountNumber, NumberToModify) == 0)
			return i;
	}
	return -1;
}

void DELETE(AccountList *List)
{
	clearScreen();
	char accountNumber[20];
	printf(COLOR_CYAN "\n===================================\n" COLOR_RESET);
	printf(COLOR_YELLOW "   Enter the account number to delete:   " COLOR_RESET);
	printf(COLOR_CYAN "\n===================================\n" COLOR_RESET);

	if (!fgets(accountNumber, sizeof(accountNumber), stdin))
	{
		clearStdin();
		printf(COLOR_RED "Input error.\n" COLOR_RESET);
		pauseScreen();
		MENU(1);
	}
	trimNewline(accountNumber);
	trimWhitespace(accountNumber);
	if (!isValid(accountNumber))
	{
		pauseScreen();
		MENU(1);
	}
	int foundIndex = findAccount(List, accountNumber);
	if (foundIndex == -1)
	{
		printf(COLOR_RED "Account not found.\n" COLOR_RESET);
		pauseScreen();
		MENU(1);
	}

	Account *acc = &List->accounts[foundIndex];
	if (acc->balance > 0.0f)
	{
		printf(COLOR_RED "Error: Deletion rejected. Account has a balance of $%.2f.\n" COLOR_RESET,
			   acc->balance);
		printf(COLOR_RED "Balance must be zero to delete this account.\n" COLOR_RESET);
		pauseScreen();
		MENU(1);
	}
	printf("\nYou are about to delete this account:\n");
	printf("Account Number: %s\n", acc->accountNumber);
	printf("Name          : %s\n", acc->name);
	printf("Email         : %s\n", acc->email);
	printf("Mobile        : %s\n", acc->mobileNumber);
	printf("Status        : %s\n", acc->status);
	printf("Balance       : %.2f\n\n", acc->balance);

	printf(COLOR_CYAN "Confirm deletion?\n" COLOR_RESET);
	printf("1. Confirm\n");
	printf("2. Discard\n");
	int choice = getIntInRange(COLOR_YELLOW "Enter choice (1-2): " COLOR_RESET, 1, 2);

	if (choice == 2)
	{
		printf(COLOR_YELLOW "[CANCELLED] No accounts deleted.\n" COLOR_RESET);
		pauseScreen();
		MENU(1);
	}
	for (int i = foundIndex; i < List->count - 1; i++)
	{
		List->accounts[i] = List->accounts[i + 1];
	}
	char filename[50];
	sprintf(filename, "%s.txt", List->accounts[foundIndex].accountNumber);
	FILE *f = fopen(filename, "w");
	fclose(f);
	List->count--;
	SAVE(List);

	printf(COLOR_GREEN "Account %s deleted successfully.\n" COLOR_RESET, accountNumber);
	pauseScreen();
	MENU(1);
}

void ADD(AccountList *List)
{
	clearScreen();
	Account newAcc = {0};
	printf(COLOR_CYAN "\n=============================\n" COLOR_RESET);
	printf(COLOR_YELLOW "     Create New Account    " COLOR_RESET);
	printf(COLOR_CYAN "\n=============================\n" COLOR_RESET);
	while (1)
	{
		printf(COLOR_YELLOW "Enter 10-digit account number: " COLOR_RESET);
		fgets(newAcc.accountNumber, sizeof(newAcc.accountNumber), stdin);
		trimNewline(newAcc.accountNumber);

		if (!isValid(newAcc.accountNumber))
		{
			continue;
		}

		if (findAccount(List, newAcc.accountNumber) != -1)
		{
			printf(COLOR_RED "Error: Account number %s already exists.\n" COLOR_RESET, newAcc.accountNumber);
			continue;
		}
		break; // Account number is valid and unique
	}
	while (1)
	{
		printf(COLOR_YELLOW "Enter account holder name: " COLOR_RESET);
		fgets(newAcc.name, sizeof(newAcc.name), stdin);
		trimNewline(newAcc.name);
		trimWhitespace(newAcc.name);
		toTitleCase(newAcc.name);

		if (strlen(newAcc.name) == 0)
		{
			printf(COLOR_RED "Error: Name cannot be empty.\n" COLOR_RESET);
			continue;
		}
		break; // Name is valid
	}
	while (1)
	{
		printf(COLOR_YELLOW "Enter email address: " COLOR_RESET);
		fgets(newAcc.email, sizeof(newAcc.email), stdin);
		trimNewline(newAcc.email);
		if (!validEmail(newAcc.email))
		{
			printf(COLOR_RED "Error: Invalid email format. Please use a format like 'name@domain.com'.\n" COLOR_RESET);
			continue;
		}
		break;
	}
	float balance;
	while (1)
	{
		printf(COLOR_YELLOW "Enter initial account balance: $" COLOR_RESET);
		if (scanf("%f", &balance) != 1 || balance < 0.0f)
		{
			printf(COLOR_RED "Error: Balance must be a non-negative number.\n" COLOR_RESET);
			clearStdin();
			continue;
		}
		break; // Balance is valid
	}
	newAcc.balance = balance;
	clearStdin();
	while (1)
	{
		printf(COLOR_YELLOW "Enter 10-15 digit mobile number: " COLOR_RESET);
		fgets(newAcc.mobileNumber, sizeof(newAcc.mobileNumber), stdin);
		trimNewline(newAcc.mobileNumber);

		int isAllDigits = 1;
		if (strlen(newAcc.mobileNumber) == 0)
			isAllDigits = 0;
		for (int i = 0; newAcc.mobileNumber[i] != '\0'; i++)
		{
			if (!isdigit(newAcc.mobileNumber[i]))
			{
				isAllDigits = 0;
				break;
			}
		}
		if (!isAllDigits || strlen(newAcc.mobileNumber) < 10 || strlen(newAcc.mobileNumber) > 15)
		{
			printf(COLOR_RED "Error: Mobile number must be 10-15 digits only.\n" COLOR_RESET);
			continue;
		}
		break;
	}

	time_t t = time(NULL);
	struct tm *tm_info = localtime(&t);
	newAcc.dateOpened.month = tm_info->tm_mon + 1;
	newAcc.dateOpened.year = tm_info->tm_year + 1900;
	newAcc.dailyWithdrawn = 0.0;
	newAcc.dailyRecieved = 0.0;
	newAcc.dailyDeposited = 0.0;
	newAcc.dailyTransferred = 0.0;
	strcpy(newAcc.status, "active");
	printf(COLOR_CYAN "\n=== Confirm New Account ===\n" COLOR_RESET);
	printf("Account Number: %s\n", newAcc.accountNumber);
	printf("Name:           %s\n", newAcc.name);
	printf("Email:          %s\n", newAcc.email);
	printf("Initial Balance:$%.2f\n", newAcc.balance);
	printf("Mobile:         %s\n", newAcc.mobileNumber);
	printf("\n");

	int choice = confirmChanges();
	if (choice == 1)
	{
		List->accounts[List->count] = newAcc;
		List->count++;
		SAVE(List);
		char filename[50];
		sprintf(filename, "%s.txt", newAcc.accountNumber);
		FILE *f = fopen(filename, "w");
		if (f)
			fclose(f);
		printf(COLOR_GREEN "\nAccount created successfully!\n" COLOR_RESET);
	}
	else
	{
		printf(COLOR_YELLOW "[CANCELLED] Changes discarded.\n" COLOR_RESET);
	}

	pauseScreen();
	MENU(1);
}

void ADVANCED_SEARCH(AccountList List)
{
	clearScreen();
	printf(COLOR_CYAN "\n=============================\n" COLOR_RESET);
	printf(COLOR_YELLOW "   Enter a keyword to search:    " COLOR_RESET);
	printf(COLOR_CYAN "\n=============================\n" COLOR_RESET);
	char keyword[50];

	fgets(keyword, sizeof(keyword), stdin);
	trimNewline(keyword);

	int found = 0;
	int i;
	for (i = 0; i < List.count; i++)
	{
		if (strstr(List.accounts[i].name, keyword) != NULL ||
			strstr(List.accounts[i].email, keyword) != NULL ||
			strstr(List.accounts[i].mobileNumber, keyword) != NULL)
		{
			printf(COLOR_WHITE "Account Number: %s\n", List.accounts[i].accountNumber);
			printf("Name: %s\n", List.accounts[i].name);
			printf("Email: %s\n", List.accounts[i].email);
			printf("Balance: %.2f $\n", List.accounts[i].balance);
			printf("Mobile Number: %s\n", List.accounts[i].mobileNumber);
			printf("Status: %s\n" COLOR_RESET, List.accounts[i].status);
			printf("\n");

			found = 1;
		}
	}

	if (!found)
	{
		printf(COLOR_RED "No accounts found matching keyword: %s\n" COLOR_RESET, keyword);
	}
	pauseScreen();
	MENU(1);
}
const char *monthName[12] =
	{
		"January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October", "November", "December"};

void WITHDRAW(AccountList *List)
{
	clearScreen();
	printf(COLOR_CYAN "\n==================================================\n" COLOR_RESET);
	printf(COLOR_YELLOW "  Enter a valid account number to withdraw from: " COLOR_RESET);
	printf(COLOR_CYAN "\n==================================================\n" COLOR_RESET);

	char accountNumber[20];
	if (!fgets(accountNumber, sizeof(accountNumber), stdin))
	{
		clearStdin(); // On error, clear buffer
		printf(COLOR_RED "Input error.\n" COLOR_RESET);
		pauseScreen();
		MENU(1);
	}
	trimNewline(accountNumber);
	int index = findAccount(List, accountNumber);
	if (index == -1)
	{
		printf(COLOR_RED "Account not found.\n" COLOR_RESET);
		pauseScreen();
		MENU(1);
	}

	if (!isActive(List->accounts[index].status))
	{
		printf(COLOR_RED "Account is not active. Cannot withdraw funds.\n" COLOR_RESET);
		pauseScreen();
		MENU(1);
	}
	float amount;
	printf("Enter amount to withdraw: $");

	if (scanf("%f", &amount) != 1)
	{
		clearStdin();
		printf(COLOR_RED "Invalid input.\n" COLOR_RESET);
		pauseScreen();
		MENU(1);
	}
	clearStdin();
	if (amount <= 0)
	{
		printf(COLOR_RED "Withdrawal amount must be positive.\n" COLOR_RESET);
		pauseScreen();
		MENU(1);
	}
	if (amount > DAILY_WITHDRAW_LIMIT)
	{
		printf(COLOR_RED "Withdrawal amount of $%.2f exceeds the single transaction limit of $%.2f\n" COLOR_RESET, amount, DAILY_WITHDRAW_LIMIT);
		pauseScreen();
		MENU(1);
	}

	if (List->accounts[index].dailyWithdrawn + amount > DAILY_WITHDRAW_LIMIT)
	{
		printf(COLOR_RED "This withdrawal would exceed your daily limit.\n" COLOR_RESET);
		printf(COLOR_YELLOW "You have $%.2f remaining for today.\n" COLOR_RESET, DAILY_WITHDRAW_LIMIT - List->accounts[index].dailyWithdrawn);
		pauseScreen();
		MENU(1);
	}

	if (amount > List->accounts[index].balance)
	{
		printf(COLOR_RED "Insufficient funds. Current balance: $%.2f\n" COLOR_RESET, List->accounts[index].balance);
		pauseScreen();
		MENU(1);
	}
	printf("\n" COLOR_YELLOW "Summary:\n" COLOR_RESET);
	printf("Account: %s\n", List->accounts[index].accountNumber);
	printf("Current Balance: $%.2f\n", List->accounts[index].balance);
	printf("Withdrawal:      $%.2f\n", amount);
	printf("New Balance:     $%.2f\n", List->accounts[index].balance - amount);

	int choice = confirmChanges();
	if (choice == 1)
	{
		List->accounts[index].balance -= amount;
		List->accounts[index].dailyWithdrawn += amount;

		SAVE(List);
		appendTransaction(List, List->accounts[index].accountNumber, "Withdraw", amount, NULL);

		printf(COLOR_GREEN "\nAmount withdrawn successfully!\n" COLOR_RESET);
	}
	else
	{
		printf(COLOR_YELLOW "[CANCELLED] Transaction discarded.\n" COLOR_RESET);
	}

	pauseScreen();
	MENU(1);
}

void QUERY(AccountList List)
{
	char accountNumber[20];
	int found = 0;
	clearScreen();
	printf(COLOR_CYAN "\n===================================\n" COLOR_RESET);
	printf(COLOR_YELLOW "  Enter account number to search:    " COLOR_RESET);
	printf(COLOR_CYAN "\n===================================\n" COLOR_RESET);

	scanf("%19s", accountNumber);
	int index = findAccount(&List, accountNumber);
	if (index != -1)
	{
		clearStdin();
		printf(COLOR_WHITE "Account Number: %s\n", List.accounts[index].accountNumber);
		printf("Name: %s\n", List.accounts[index].name);
		printf("Email: %s\n", List.accounts[index].email);
		printf("Balance: %.2f $\n", List.accounts[index].balance);
		printf("Mobile Number: %s\n", List.accounts[index].mobileNumber);
		printf("Status: %s\n" COLOR_RESET, List.accounts[index].status);
		found = 1;
	}
	if (!found)
	{
		printf(COLOR_RED "No account found with account number: %s\n" COLOR_RESET, accountNumber);
	}
	pauseScreen();
	MENU(1);
}

AccountList LOAD(void)
{
	AccountList List;
	List.count = 0;

	FILE *fileaccounts = fopen("accounts.txt", "r");
	if (!fileaccounts)
	{
		return List;
	}

	char line[200];

	while (fgets(line, sizeof(line), fileaccounts))
	{
		char *token;
		line[strcspn(line, "\n")] = '\0';

		token = strtok(line, ",");
		if (!token)
			continue;
		strcpy(List.accounts[List.count].accountNumber, token);

		token = strtok(NULL, ",");
		if (!token)
			continue;
		strcpy(List.accounts[List.count].name, token);

		token = strtok(NULL, ",");
		if (!token)
			continue;
		strcpy(List.accounts[List.count].email, token);

		token = strtok(NULL, ",");
		if (!token)
			continue;
		List.accounts[List.count].balance = atof(token);

		token = strtok(NULL, ",");
		if (!token)
			continue;
		strcpy(List.accounts[List.count].mobileNumber, token);

		token = strtok(NULL, ",");
		if (!token)
			continue;
		sscanf(token, "%d-%d",
			   &List.accounts[List.count].dateOpened.month,
			   &List.accounts[List.count].dateOpened.year);

		token = strtok(NULL, ",");
		if (!token)
			continue;
		strcpy(List.accounts[List.count].status, token);

		List.count++;
	}

	fclose(fileaccounts);
	resetDailyLimits(&List);
	createfiles(&List);
	return List;
}

void DEPOSIT(AccountList *List)
{
	clearScreen();
	printf(COLOR_CYAN "\n=====================================\n" COLOR_RESET);
	printf(COLOR_YELLOW "  Enter account number to deposit: " COLOR_RESET);
	printf(COLOR_CYAN "\n=====================================\n" COLOR_RESET);

	char accountnumber[20];
	if (!fgets(accountnumber, sizeof(accountnumber), stdin))
	{
		clearStdin();
		printf(COLOR_RED "Input error.\n" COLOR_RESET);
		pauseScreen();
		MENU(1);
	}

	trimNewline(accountnumber);
	trimWhitespace(accountnumber);
	if (!isValid(accountnumber))
	{
		pauseScreen();
		MENU(1);
	}
	int index = findAccount(List, accountnumber);
	if (index == -1)
	{
		printf(COLOR_RED "Account not found.\n" COLOR_RESET);
		pauseScreen();
		MENU(1);
	}
	if (!isActive(List->accounts[index].status))
	{
		printf(COLOR_RED "Warning: This account is inactive. Cannot deposit.\n" COLOR_RESET);
		pauseScreen();
		MENU(1);
	}
	float depositAmount;
	printf("Enter amount to deposit: $");
	while (1)
	{
		if (scanf("%f", &depositAmount) != 1)
		{
			printf(COLOR_RED "Invalid input. Please enter a number.\n" COLOR_RESET);
			clearStdin();
			continue;
		}

		if (depositAmount < 0)
		{
			printf(COLOR_RED "Deposit amount cannot be negative. Please re-enter.\n" COLOR_RESET);
			continue;
		}

		if (depositAmount > 10000.0)
		{
			printf(COLOR_RED "Max amount per transaction is $10,000. Please enter a smaller amount.\n" COLOR_RESET);
			continue;
		}

		break;
	}
	clearStdin();
	printf("\n" COLOR_YELLOW "Summary:\n" COLOR_RESET);
	printf("Account: %s\n", List->accounts[index].accountNumber);
	printf("Current Balance: $%.2f\n", List->accounts[index].balance);
	printf("Deposit Amount:  $%.2f\n", depositAmount);
	printf("New Balance:     $%.2f\n", List->accounts[index].balance + depositAmount);

	int choice = confirmChanges();
	if (choice == 1)
	{
		List->accounts[index].dailyDeposited += depositAmount;
		List->accounts[index].balance += depositAmount;
		SAVE(List);
		appendTransaction(List, List->accounts[index].accountNumber, "Deposit", depositAmount, NULL);
		printf(COLOR_GREEN "\nAmount deposited successfully!\n" COLOR_RESET);
	}
	else
	{
		printf(COLOR_YELLOW "[CANCELLED] Changes discarded.\n" COLOR_RESET);
	}

	pauseScreen();
	MENU(1);
}

void TRANSFER(AccountList *List)
{

	float amount;
	char sendernum[20], recievernum[20];
	clearScreen();
	printf(COLOR_CYAN "\n============================\n" COLOR_RESET);
	printf(COLOR_YELLOW "   TRANSFER UTILITY   " COLOR_RESET);
	printf(COLOR_CYAN "\n============================\n" COLOR_RESET);

	printf("Enter the sender's account number: ");
	scanf("%s", sendernum);
	clearStdin();
	printf("Enter the reciever's account number: ");
	scanf("%s", recievernum);
	clearStdin();
	int index1 = findAccount(List, sendernum);
	int index2 = findAccount(List, recievernum);
	int active1 = isActive(List->accounts[index1].status);
	int active2 = isActive(List->accounts[index2].status);
	while (1)
	{
		if (index1 == -1 && index2 == -1)
		{
			printf(COLOR_RED "account numbers of the sender and reciever doesn't exist\n" COLOR_RESET);
			printf("Returning to Menu.\n");
			pauseScreen();
			MENU(1);
		}
		if (index1 == -1)
		{
			printf(COLOR_RED "account number of the sender doesn't exist\n" COLOR_RESET);
			printf("Returning to Menu.\n");
			pauseScreen();
			MENU(1);
		}
		if (index2 == -1)
		{
			printf(COLOR_RED "account number of the reciever doesn't exist\n" COLOR_RESET);
			printf("Returning to Menu.\n");
			pauseScreen();
			MENU(1);
		}
		if (active1 == 0 && active2 == 0)
		{
			printf(COLOR_RED "accounts of the sender and the reciever are inactive. can not complete the transfering process\n" COLOR_RESET);
			printf("Returning to Menu.\n");
			pauseScreen();
			MENU(1);
		}
		if (active1 == 0)
		{
			printf(COLOR_RED "account of the sender is inactive. can not complete the transfering process\n" COLOR_RESET);
			printf("Returning to Menu.\n");
			pauseScreen();
			MENU(1);
		}
		if (active2 == 0)
		{
			printf(COLOR_RED "account of the reciever is inactive. can not complete the transfering process\n" COLOR_RESET);
			printf("Returning to Menu.\n");
			pauseScreen();
			MENU(1);
		}
		printf("enter the amount of money to transfer\n");
		scanf("%f", &amount);
		while (amount < 0)
		{
			clearStdin();
			printf(COLOR_RED "Warning: This value is Invalid." COLOR_RESET);
			pauseScreen();
			printf("Would you like to:\n");
			printf("1.Re-enter amount\n");
			printf("2.Exit program\n");
			int choice;
			choice = getIntInRange("Enter choice (1-2): ", 1, 2);
			switch (choice)
			{
			case 1:
				printf("Re-enter amount: ");
				scanf("%f", &amount);
				if (amount < 0)
				{
					printf(COLOR_RED "Invalid Amount.Returning to Menu\n" COLOR_RESET);
					pauseScreen();
					MENU(1);
				}
				break;
			case 2:
				QUIT();
				break;
			default:
				printf(COLOR_RED "Invalid choice.\n");
				printf("Returning to menu.\n" COLOR_RESET);
				pauseScreen();
				MENU(1);
				break;
			}
			if (amount > DAILY_WITHDRAW_LIMIT)
			{
				printf(COLOR_RED "Max amount per transaction is 10,000 $\n" COLOR_RESET);
				printf("Can not complete transfering process. Returning to Menu.\n");
				pauseScreen();
				MENU(1);
			}
			if (amount + (List->accounts[index1].dailyTransferred) > DAILY_TRANSFER_LIMIT)
			{
				printf(COLOR_RED "The daily transfer limit is 50,000.You have %.3f remaining to transfer\n" COLOR_RESET, DAILY_TRANSFER_LIMIT - (List->accounts[index1].dailyTransferred));
				printf("Can not complete transfering process. Returning to Menu.\n");
				pauseScreen();
				MENU(1);
			}
			if (amount > List->accounts[index1].balance)
			{
				printf(COLOR_RED "Insufficient funds. your current balance is %.3f\n" COLOR_RESET, List->accounts[index1].balance);
				printf("Can not complete transfering process. Returning to Menu.\n");
				pauseScreen();
				MENU(1);
			}
			List->accounts[index1].balance -= amount;
			List->accounts[index2].balance += amount;
			List->accounts[index1].dailyTransferred += amount;
			List->accounts[index1].dailyTransferred += amount;
			List->accounts[index1].dailyRecieved -= amount;
			break;
		}
		clearStdin();
		int choice = confirmChanges();
		if (choice == 1)
		{
			SAVE(List);
			char detailsOut[50], detailsIn[50];
			sprintf(detailsOut, "To: %s", List->accounts[index2].accountNumber);
			sprintf(detailsIn, "From: %s", List->accounts[index1].accountNumber);
			appendTransaction(List, List->accounts[index1].accountNumber, "Transfer Out", amount, detailsOut);
			appendTransaction(List, List->accounts[index2].accountNumber, "Transfer In", amount, detailsIn);
			printf(COLOR_GREEN "\nAmount transferred successfully!\n" COLOR_RESET);
		}
		else
		{
			*List = LOAD();
			printf(COLOR_YELLOW "[CANCELLED] Changes discarded.\n" COLOR_RESET);
		}
		pauseScreen();
		MENU(1);
	}
}
void MODIFY(AccountList *List)
{
	int index = -1;
	while (1)
	{
		clearScreen();
		printf(COLOR_CYAN "\n=================================================\n" COLOR_RESET);
		printf(COLOR_YELLOW "  Enter account number to modify (or type 'cancel' to exit): " COLOR_RESET);
		printf(COLOR_CYAN "\n=================================================\n" COLOR_RESET);
		char numberToModify[20];
		if (!fgets(numberToModify, sizeof(numberToModify), stdin))
		{
			continue;
		}
		trimNewline(numberToModify);
		trimWhitespace(numberToModify);
		for (int i = 0; numberToModify[i] != '\0'; i++)
		{
			numberToModify[i] = tolower((unsigned char)numberToModify[i]);
		}
		if (strcmp(numberToModify, "cancel") == 0)
		{
			printf("Operation cancelled. Returning to main menu.\n");
			pauseScreen();
			MENU(1);
			return;
		}
		index = findAccount(List, numberToModify);

		if (index != -1)
		{
			break;
		}
		printf(COLOR_RED "\nAccount number '%s' does not exist.\n" COLOR_RESET, numberToModify);
		printf(COLOR_YELLOW "Please try again, or type 'cancel' to return to the menu.\n" COLOR_RESET);
		pauseScreen();
	}
	clearScreen();
	printf(COLOR_GREEN "Account Found: %s - %s\n" COLOR_RESET, List->accounts[index].accountNumber, List->accounts[index].name);

	printf(COLOR_YELLOW "\n=== What would you like to modify? ===\n");
	printf("1. Name          (Current: %s)\n", List->accounts[index].name);
	printf("2. Email Address (Current: %s)\n", List->accounts[index].email);
	printf("3. Mobile Number (Current: %s)\n", List->accounts[index].mobileNumber);
	printf("4. Cancel Modification\n");

	int choice = getIntInRange(COLOR_YELLOW "Enter your choice (1-4): " COLOR_RESET, 1, 4);

	if (choice == 4)
	{
		printf("Modification cancelled. Returning to menu.\n");
		pauseScreen();
		MENU(1);
		return;
	}

	char newValue[100];
	int modified = 0;

	switch (choice)
	{
	case 1: // Modify Name
		printf("Enter new name: ");
		fgets(newValue, sizeof(newValue), stdin);
		trimNewline(newValue);
		trimWhitespace(newValue);

		if (strlen(newValue) > 0)
		{
			toTitleCase(newValue);
			strcpy(List->accounts[index].name, newValue);
			printf(COLOR_GREEN "Name updated.\n" COLOR_RESET);
			modified = 1;
		}
		else
		{
			printf(COLOR_RED "Name cannot be empty. Change cancelled.\n" COLOR_RESET);
		}
		break;

	case 2:
		printf("Enter new email address: ");
		fgets(newValue, sizeof(newValue), stdin);
		trimNewline(newValue);
		if (validEmail(newValue))
		{
			strcpy(List->accounts[index].email, newValue);
			printf(COLOR_GREEN "Email address updated successfully.\n" COLOR_RESET);
			modified = 1;
		}
		else
		{
			printf(COLOR_RED "Invalid email format. Change cancelled.\n" COLOR_RESET);
		}
		break;
	case 3: // Modify Mobile Number
		printf("Enter new mobile number (10-15 digits): ");
		fgets(newValue, sizeof(newValue), stdin);
		trimNewline(newValue);

		int valid = 1;
		if (strlen(newValue) < 10 || strlen(newValue) > 15)
			valid = 0;
		for (int i = 0; newValue[i] != '\0'; i++)
		{
			if (!isdigit(newValue[i]))
			{
				valid = 0;
				break;
			}
		}
		if (valid)
		{
			strcpy(List->accounts[index].mobileNumber, newValue);
			printf(COLOR_GREEN "Mobile number updated.\n" COLOR_RESET);
			modified = 1;
		}
		else
		{
			printf(COLOR_RED "Invalid mobile number (must be 10-15 digits). Change cancelled.\n" COLOR_RESET);
		}
		break;
	}
	if (modified)
	{
		int choiceConfirm = confirmChanges();
		if (choiceConfirm == 1)
		{
			SAVE(List);
			printf(COLOR_GREEN "\nAccount modified successfully!\n" COLOR_RESET);
		}
		else
		{
			*List = LOAD(); // Revert by reloading from file
			printf(COLOR_YELLOW "[CANCELLED] Changes discarded.\n" COLOR_RESET);
		}
	}
	pauseScreen();
	MENU(1);
}

void CHANGE_STATUS(AccountList *List)
{
	clearScreen();

	char NumberToChange[20];
	printf(COLOR_CYAN "\n===================================\n" COLOR_RESET);
	printf(COLOR_YELLOW "Enter account number to change status: " COLOR_RESET);
	printf(COLOR_CYAN "\n===================================\n" COLOR_RESET);

	fgets(NumberToChange, sizeof(NumberToChange), stdin);
	trimNewline(NumberToChange);
	if (!isValid(NumberToChange))
	{

		printf(COLOR_RED "Invalid account number format.\n" COLOR_RESET);
		pauseScreen();
		return;
	}
	int index = findAccount(List, NumberToChange);
	if (index == -1)
	{
		printf(COLOR_RED "Account does not exist.\n" COLOR_RESET);
		pauseScreen();
		return;
	}
	printf("\nAccount Number: %s\n", List->accounts[index].accountNumber);
	printf("Current Status: %s\n", List->accounts[index].status);
	printf("\nChoose new Account Status:\n");
	printf("1. active\n");
	printf("2. inactive\n");

	int choice1;
	choice1 = getIntInRange(COLOR_YELLOW "Enter your choice (1-2): " COLOR_RESET, 1, 2);
	int active = isActive(List->accounts[index].status);
	if (choice1 == 1)
	{
		if (active)
		{
			printf(COLOR_YELLOW "Account is already active.\n" COLOR_RESET);
			pauseScreen();
			MENU(1);
		}
		strcpy(List->accounts[index].status, "active");
	}
	else if (choice1 == 2)
	{
		if (!active)
		{
			printf(COLOR_YELLOW "Account is already inactive.\n" COLOR_RESET);
			pauseScreen();
			MENU(1);
		}
		strcpy(List->accounts[index].status, "inactive");
	}
	else
	{
		printf(COLOR_RED "Invalid choice. Must be 1 or 2.\n" COLOR_RESET);
		pauseScreen();
		MENU(1);
	}
	int choice = confirmChanges();
	if (choice == 1)
	{
		SAVE(List);
		printf(COLOR_GREEN "\nStatus changed successfully!\n" COLOR_RESET);
	}
	else
	{
		*List = LOAD();
		printf(COLOR_YELLOW "[CANCELLED] Changes discarded.\n" COLOR_RESET);
	}

	pauseScreen();
	MENU(1);
}

void SortByName(AccountList *list)
{
	Account temp;
	int i, j;
	for (i = 0; i < list->count - 1; i++)
	{
		for (j = 0; j < (list->count - i - 1); j++)
		{
			if (strcmp(list->accounts[j].name, list->accounts[j + 1].name) > 0)
			{
				temp = list->accounts[j];
				list->accounts[j] = list->accounts[j + 1];
				list->accounts[j + 1] = temp;
			}
		}
	}
}

void SortByDate(AccountList *list)
{
	Account temp;
	int i, j;
	for (i = 0; i < list->count - 1; i++)
	{
		for (j = 0; j < (list->count - i - 1); j++)
		{
			if (list->accounts[j].dateOpened.year > list->accounts[j + 1].dateOpened.year ||
				(list->accounts[j].dateOpened.year == list->accounts[j + 1].dateOpened.year &&
				 list->accounts[j].dateOpened.month > list->accounts[j + 1].dateOpened.month))
			{
				temp = list->accounts[j];
				list->accounts[j] = list->accounts[j + 1];
				list->accounts[j + 1] = temp;
			}
		}
	}
}
void SortByBalance(AccountList *list)
{
	Account temp;
	int i, j;
	for (i = 0; i < list->count - 1; i++)
	{
		for (j = 0; j < (list->count - i - 1); j++)
		{
			if (list->accounts[j].balance > list->accounts[j + 1].balance)
			{
				temp = list->accounts[j];
				list->accounts[j] = list->accounts[j + 1];
				list->accounts[j + 1] = temp;
			}
		}
	}
}
void SortByStatus(AccountList *list)
{
	Account temp;
	int i, j;
	for (i = 0; i < list->count - 1; i++)
	{
		for (j = 0; j < (list->count - i - 1); j++)
		{
			if (strcmp(list->accounts[j].status, "inactive") == 0 && strcmp(list->accounts[j + 1].status, "active") == 0)
			{
				temp = list->accounts[j];
				list->accounts[j] = list->accounts[j + 1];
				list->accounts[j + 1] = temp;
			}
		}
	}
}
void PRINT(AccountList *List)
{
	clearScreen();
	AccountList list = LOAD();
	int choice;
	clearScreen();
	printf(COLOR_CYAN "\n===============================\n" COLOR_RESET);
	printf(COLOR_YELLOW "     SORT ACCOUNTS     " COLOR_RESET);
	printf(COLOR_CYAN "\n===============================\n" COLOR_RESET);

	printf("Sort By:\n");
	printf("1. Name \n");
	printf("2. Date \n");
	printf("3. Balance \n");
	printf("4. Status \n");
	choice = getIntInRange(COLOR_YELLOW "Enter your choice (1-4): " COLOR_RESET, 1, 4);
	switch (choice)
	{
	case 1:
		SortByName(&list);
		break;
	case 2:
		SortByDate(&list);
		break;
	case 3:
		SortByBalance(&list);
		break;
	case 4:
		SortByStatus(&list);
		break;
	default:
		printf(COLOR_RED "Invalid choice" COLOR_RESET);
		pauseScreen();
		MENU(1);
		break;
	}
	int i;
	for (i = 0; i < list.count; i++)
	{
		printf("\n");
		printf("Account Number: %s\n", list.accounts[i].accountNumber);
		printf("Name: %s\n", list.accounts[i].name);
		printf("E-mail: %s\n", list.accounts[i].email);
		printf("Balance: %.2f $\n", list.accounts[i].balance);
		printf("Mobile: %s\n", list.accounts[i].mobileNumber);
		int m = List->accounts[i].dateOpened.month;
		int y = List->accounts[i].dateOpened.year;
		if (m >= 1 && m <= 12)
			printf("Date Opened: %s %d\n", monthName[m - 1], y);
		else
			printf("Date Opened: <invalid month> %d\n", y);

		printf("Status: %s\n", list.accounts[i].status);
	}
	pauseScreen();
	MENU(1);
}
void createfiles(AccountList *List)
{
	int i;
	char filename[50];

	for (i = 0; i < List->count; i++)
	{
		sprintf(filename, "%s.txt", List->accounts[i].accountNumber);

		FILE *f = fopen(filename, "a");
		if (f == NULL)
		{
			printf(COLOR_RED "Error creating file %s\n" COLOR_RESET, filename);
			continue;
		}
		fclose(f);
	}
}
void appendTransaction(AccountList *List, char *accNumber,
					   char *type, float amount, char *details)
{
	int index = findAccount(List, accNumber);
	if (index == -1)
	{
		printf(COLOR_RED "Account not found.\n" COLOR_RESET);
		pauseScreen();
		MENU(1);
	}
	char filename[100];
	sprintf(filename, "%s.txt", List->accounts[index].accountNumber);

	FILE *f = fopen(filename, "a");
	if (f == NULL)
	{
		printf(COLOR_RED "Error opening transaction file.\n" COLOR_RESET);
		pauseScreen();
		MENU(1);
	}
	if (details == NULL)
		fprintf(f, "%s | %.2f\n", type, amount);
	else
		fprintf(f, "%s | %.2f | %s\n", type, amount, details);

	fclose(f);
}
void REPORT(AccountList *List)
{
	clearScreen();
	char NumberToReport[20];
	printf(COLOR_CYAN "\n===================================\n" COLOR_RESET);
	printf(COLOR_YELLOW "Enter account number: " COLOR_RESET);
	printf(COLOR_CYAN "\n===================================\n" COLOR_RESET);
	scanf("%s", NumberToReport);

	int index = findAccount(List, NumberToReport);
	if (index == -1)
	{
		printf("Account not found.\n");
		pauseScreen();
		MENU(1);
	}
	char filename[50];
	char lines[200][300];
	int count = 0, i;

	sprintf(filename, "%s.txt", List->accounts[index].accountNumber);

	FILE *f = fopen(filename, "r");
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	if (size == 0)
	{
		clearStdin();
		printf("No transaction history for this account.\n");
		pauseScreen();
		MENU(1);
		return;
	}
	rewind(f);
	while (fgets(lines[count], 300, f) != NULL)
	{
		count++;
	}
	fclose(f);
	printf(COLOR_YELLOW "\nLast transactions for account %s:\n" COLOR_RESET,
		   List->accounts[index].accountNumber);

	int start = (count > 5) ? count - 5 : 0;
	for (i = start; i < count; i++)
	{
		printf("%s", lines[i]);
	}
	clearStdin();
	pauseScreen();
	MENU(1);
}
void DELETEMULTIPLE(AccountList *List)
{
	clearScreen();
	int choice;
	int deletedCount = 0;

	printf(COLOR_CYAN "\n=============================\n" COLOR_RESET);
	printf(COLOR_YELLOW "   BULK DELETE UTILITY   " COLOR_RESET);
	printf(COLOR_CYAN "\n=============================\n" COLOR_RESET "\n");
	printf("1. Delete by Creation Date (MM-YYYY) & Zero Balance\n");
	printf("2. Delete Inactive, Zero-Balance Accounts (Older than 3 months)\n");
	printf("3. Cancel\n");
	choice = getIntInRange(COLOR_YELLOW "Enter choice (1-3): " COLOR_RESET, 1, 3);
	clearStdin();
	if (choice == 1)
	{
		char dateStr[20];
		int targetMonth, targetYear;
		printf(COLOR_YELLOW "\nEnter Date (Format MM-YYYY): " COLOR_RESET);
		fgets(dateStr, sizeof(dateStr), stdin);
		trimNewline(dateStr);
		if (!parseTargetDate(dateStr, &targetMonth, &targetYear))
		{
			printf(COLOR_RED "Error: Invalid date format. Please use MM-YYYY.\n" COLOR_RESET);
			pauseScreen();
			MENU(1);
		}
		printf(COLOR_CYAN "\nSearching for accounts created in %02d-%d with a ZERO balance...\n" COLOR_RESET, targetMonth, targetYear);
		pauseScreen();
		for (int i = List->count - 1; i >= 0; i--)
		{
			Account *acc = &List->accounts[i];
			if (acc->dateOpened.month == targetMonth &&
				acc->dateOpened.year == targetYear &&
				acc->balance == 0.0f)
			{
				for (int j = i; j < List->count - 1; j++)
				{
					List->accounts[j] = List->accounts[j + 1];
				}
				char filename[50];
				sprintf(filename, "%s.txt", List->accounts[i].accountNumber);
				FILE *f = fopen(filename, "w");
				fclose(f);
				List->count--;
				deletedCount++;
			}
		}
		if (deletedCount == 0)
		{
			printf(COLOR_RED "\n[RESULT] No accounts found for the given date with a zero balance.\n" COLOR_RESET);
		}
		else
		{
			printf(COLOR_GREEN "\n[FOUND] %d account(s) with a zero balance matched the date.\n" COLOR_RESET, deletedCount);

			if (confirmChanges() == 1)
			{
				SAVE(List);
				printf(COLOR_GREEN "[SUCCESS] %d accounts were permanently deleted.\n" COLOR_RESET, deletedCount);
			}
			else
			{
				*List = LOAD();
				printf(COLOR_YELLOW "[CANCELLED] Changes discarded. No accounts were deleted.\n" COLOR_RESET);
			}
		}
	}
	else if (choice == 2)
	{
		printf(COLOR_YELLOW "\nScanning for accounts that are:\n" COLOR_RESET);
		printf(" 1. Inactive\n");
		printf(" 2. Have a zero balance\n");
		printf(" 3. Are older than 3 months\n\n");
		pauseScreen();

		time_t t = time(NULL);
		struct tm *tm_info = localtime(&t);
		int currentYear = tm_info->tm_year + 1900;
		int currentMonth = tm_info->tm_mon + 1;
		long totalCurrentMonths = currentYear * 12 + currentMonth;

		for (int i = List->count - 1; i >= 0; i--)
		{
			Account *acc = &List->accounts[i];
			long totalAccountMonths = acc->dateOpened.year * 12 + acc->dateOpened.month;

			int isInactive = (strcmp(acc->status, "inactive") == 0);
			int isZeroBalance = (acc->balance == 0.0f);
			int isOldEnough = ((totalCurrentMonths - totalAccountMonths) > 3);

			if (isInactive && isZeroBalance && isOldEnough)
			{
				for (int j = i; j < List->count - 1; j++)
				{
					List->accounts[j] = List->accounts[j + 1];
				}
				List->count--;
				deletedCount++;
			}
		}
		if (deletedCount == 0)
		{
			printf(COLOR_RED "\n[RESULT] No accounts met all three criteria for deletion.\n" COLOR_RESET);
		}
		else
		{
			printf(COLOR_GREEN "\n[FOUND] %d eligible accounts found.\n" COLOR_RESET, deletedCount);
			if (confirmChanges() == 1)
			{
				SAVE(List);
				printf(COLOR_GREEN "[SUCCESS] %d inactive accounts purged.\n" COLOR_RESET, deletedCount);
			}
			else
			{
				*List = LOAD();
				printf(COLOR_YELLOW "[CANCELLED] Changes discarded.\n" COLOR_RESET);
			}
		}
	}
	else if (choice == 3)
	{
		printf("Returning to menu...\n");
	}
	pauseScreen();
	MENU(1);
}

int LOGIN(void)
{
	while (1) // outer loop: keep showing login menu until success or exit
	{
		clearScreen();
		printf(COLOR_CYAN "\n=============================\n" COLOR_RESET);
		printf(COLOR_YELLOW "   WELCOME TO BANKING SYSTEM   " COLOR_RESET);
		printf(COLOR_CYAN "\n=============================\n" COLOR_RESET);
		printf("\nSelect an option:\n");
		printf("1. Login\n");
		printf("2. Exit\n\n");

		// ---- READ MENU CHOICE SAFELY (1 or 2) ----
		int choiceOne = getIntInRange("Enter choice (1-2): ", 1, 2);

		if (choiceOne == 2)
		{
			printf(COLOR_YELLOW "Exiting program.\n" COLOR_RESET);
			exit(0);
		}

		// ---- LOGIN PATH (choiceOne == 1) ----
		FILE *fileUsers = fopen("users.txt", "r");
		if (!fileUsers)
		{
			printf(COLOR_RED "Error: could not open users.txt.\n" COLOR_RESET);
			printf("Make sure the file exists and is in the correct folder.\n");
			pauseScreen();
			// You can either retry or exit; here we just retry the menu
			continue;
		}

		char username[50], password[50];

		// Read username safely
		while (1)
		{
			printf("Username: ");
			if (!fgets(username, sizeof(username), stdin))
				continue;

			trimNewline(username);
			trimWhitespace(username);

			if (username[0] == '\0')
			{
				printf(COLOR_RED "Username cannot be empty.\n" COLOR_RESET);
				continue;
			}
			break;
		}

		// Read password safely
		while (1)
		{
			printf("Password: ");
			if (!fgets(password, sizeof(password), stdin))
				continue;

			trimNewline(password);
			trimWhitespace(password);

			if (password[0] == '\0')
			{
				printf(COLOR_RED "Password cannot be empty.\n" COLOR_RESET);
				continue;
			}
			break;
		}

		int loggedIn = 0;
		char fileUsername[50], filePassword[50];

		// ---- CHECK CREDENTIALS AGAINST users.txt ----
		while (fscanf(fileUsers, "%49s %49s", fileUsername, filePassword) == 2)
		{
			if (strcmp(username, fileUsername) == 0 &&
				strcmp(password, filePassword) == 0)
			{
				loggedIn = 1;
				break;
			}
		}

		fclose(fileUsers);

		if (loggedIn)
		{
			printf(COLOR_GREEN "Login successful.\n" COLOR_RESET);
			pauseScreen();
			return 1; // success
		}
		else
		{
			printf(COLOR_RED "Invalid username or password.\n" COLOR_RESET);

			// Ask if user wants to try again or exit, using getIntInRange
			printf("\nWhat would you like to do?\n");
			printf("1. Try again\n");
			printf("2. Exit\n");

			int retryChoice = getIntInRange("Enter choice (1-2): ", 1, 2);

			if (retryChoice == 2)
			{
				printf(COLOR_YELLOW "Exiting program.\n" COLOR_RESET);
				exit(0);
			}
			// else retry (loop continues)
		}
	}
}
int MENU(int loggedIn)
{
	if (loggedIn == 1)
	{
		clearScreen();
		LOAD();
		printf(COLOR_GREEN "System loaded successfully.\n" COLOR_RESET);
		printf("Select a service to continue.\n");
		printf(COLOR_WHITE "[Enter 1-13]\n" COLOR_RESET);
		printf(COLOR_WHITE "1. ADD\n");
		printf("2. DELETE \n");
		printf("3. MODIFY \n");
		printf("4. SEARCH\n");
		printf("5. ADVANCED SEARCH \n");
		printf("6. CHANGE STATUS \n");
		printf("7. WITHDRAW \n");
		printf("8. DEPOSIT \n");
		printf("9. TRANSFER \n");
		printf("10. REPORT \n");
		printf("11. PRINT \n");
		printf("12. DELETE MULTIPLE \n");
		printf("13. QUIT \n" COLOR_RESET);
		int serviceChoice;
		serviceChoice = getIntInRange("Enter your choice:(1-13) ", 1, 13);
		if (serviceChoice >= 1 && serviceChoice < 13)
		{
			if (serviceChoice == 1)
			{
				AccountList List = LOAD();
				ADD(&List);
			}
			else if (serviceChoice == 2)
			{
				AccountList List = LOAD();
				DELETE(&List);
			}
			else if (serviceChoice == 3)
			{
				AccountList List = LOAD();
				MODIFY(&List);
			}
			else if (serviceChoice == 4)
			{
				AccountList List = LOAD();
				QUERY(List);
			}
			else if (serviceChoice == 5)
			{
				AccountList List = LOAD();
				ADVANCED_SEARCH(List);
			}
			else if (serviceChoice == 6)

			{
				AccountList List = LOAD();
				CHANGE_STATUS(&List);
			}
			else if (serviceChoice == 7)

			{
				AccountList List = LOAD();
				WITHDRAW(&List);
			}
			else if (serviceChoice == 8)

			{
				AccountList List = LOAD();
				DEPOSIT(&List);
			}
			else if (serviceChoice == 9)

			{
				AccountList List = LOAD();
				TRANSFER(&List);
			}
			else if (serviceChoice == 10)

			{
				AccountList List = LOAD();
				REPORT(&List);
			}
			else if (serviceChoice == 11)

			{
				AccountList List = LOAD();
				PRINT(&List);
				MENU(1);
			}
			else if (serviceChoice == 12)
			{
				AccountList List = LOAD();
				DELETEMULTIPLE(&List);
			}
		}
		else if (serviceChoice == 13)
		{
			QUIT();
		}
		else
		{
			printf("Invalid service choice. Return to menu or exit\n");
			printf("1. Return to Menu\n");
			printf("2. Exit\n");
			int postLoginChoice;
			scanf("%d", &postLoginChoice);
			clearStdin();
			if (postLoginChoice == 1)
			{
				return MENU(loggedIn);
			}
			else if (postLoginChoice == 2)
			{
				QUIT();
			}
			else
			{
				printf(COLOR_RED "Invalid choice.\n" COLOR_RESET);
				QUIT();
			}
		}
	}
	else
	{
		printf("Login failed. Returning to login\n");
		return LOGIN();
	}
	return 0;
}
#endif // FUNCTIONS_H_INCLUDED