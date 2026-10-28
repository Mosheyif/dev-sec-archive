/*********************************************************
*
* Description: Test runner and interactive client for the
*              authenticator module
* Author: Moshe Yifrah
*
* Compile with: make
*
* Run by: ./authenticator_test
*
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "authenticator.h"

#define MAX_BUFFER 100

void GetLineSafe(char *buffer, size_t size);
int test_pam();

void GetPasswordHidden(char *buffer, size_t size)
{
	struct termios old_t, new_t;

	tcgetattr(STDIN_FILENO, &old_t);
	new_t = old_t;

	new_t.c_lflag &= ~ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &new_t);

	GetLineSafe(buffer, size);

	tcsetattr(STDIN_FILENO, TCSANOW, &old_t);

	printf("\n"); 

}

int main(void)
{
	int choice = -1;
	int auth_choice = -1;
	char user_name[MAX_BUFFER];
	char password[MAX_BUFFER];
	char choice_str[16];

	while (choice != 0)
	{
		printf("\n--- Authentication System Menu ---\n");
		printf("1. Add User\n");
		printf("2. Delete User\n");
		printf("3. Authenticate User\n");
		printf("0. Exit\n");
		printf("Enter choice: ");

		GetLineSafe(choice_str, sizeof(choice_str));
		choice = atoi(choice_str);

		switch (choice)
		{
		    case 1:
		        printf("Enter username: ");
		        GetLineSafe(user_name, MAX_BUFFER);
		        printf("Enter password: ");
		        GetPasswordHidden(password, MAX_BUFFER);
		        
		        if (AddUser(user_name, password))
		            printf("[+] User added successfully!\n");
		        else
		            printf("[-] Failed to add user\n");
		        break;

		    case 2:
		    	printf("Enter username: ");
		        GetLineSafe(user_name, MAX_BUFFER);
		        printf("Enter password: ");
		        GetPasswordHidden(password, MAX_BUFFER);
		    
			    if (DeleteUser(user_name, password))
					{
					    printf("[+] User deleted successfully!\n");
					}
				else
			       {
			       		printf("[-] Failed to delete user\n");
			    	}
		        
		        break;

		    case 3:
		    	printf("Insert authenticator choice:\n1. The regular authenticator\n2. Use PAM\n");
		    	scanf("%d", &auth_choice);
		    	getchar();
		    	if (2 == auth_choice)
		    	{
		    		test_pam();
		    	}
		    	
		    	else if (1 == auth_choice)
		    	{
				  	printf("Enter username: ");
				    GetLineSafe(user_name, MAX_BUFFER);
				    printf("Enter password: ");
				    GetPasswordHidden(password, MAX_BUFFER);
				    
				    if (Authenticate(user_name, password))
				        printf("[+] Authentication successful!\n");
				    else
				        printf("[-] Authentication failed.\n");
				}
				else
				{
					printf("Invalid choice\n");
				}
		        break;

		    case 0:
		        printf("Exiting system...\n");
		        break;
		        
		    default:
		        printf("Invalid choice\n");
		}
	}

	return 0;
}
