/*********************************************************
*
* Description: Implemention of a logger
* Author: Moshe Yifrah
*
*********************************************************/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>  /* printf and files functions */
#include <string.h> /* strlen, strcspn, strncmp */
#include <stdlib.h> /* free & exit */

/*********************************************************/

const size_t NUM_OF_CMD = 5;

typedef enum Status_ 
{
	SUCCESS,
	ERROR,
	EXIT
} Status;

typedef struct Command 
{
	char* cmd_id;
	Status (*IsMatch)(char*, char*);
	Status (*exec)(char*, char*);
} commands;

/*********************************************************/

int Logger(char*);
Status SelectCmd(char*, char*);
Status AppendLine(char*, char*);
Status CountLines(char*, char*);
Status PreAppend(char*, char*);
Status RemoveFile(char*, char*);
Status Exit(char*, char*);
Status StrMatch(char*, char*);
Status AppendMatch(char*, char*);
Status Append(char*, char*);
Status ReturnTrue(char*, char*);
void Verify(int);

/*********************************************************/

int Logger(char* file_name)
{
	char* line = NULL;
	int status = SUCCESS;
	size_t size = 0;

	while (status == SUCCESS)
	{
		printf("Enter a line\n");
		Verify (getline(&line, &size, stdin) < 0);
		status = SelectCmd(file_name, line);
	}

	free(line);
	line = NULL;

	return SUCCESS;
}

/*********************************************************/

Status SelectCmd(char* file_name, char* line)
{
	commands command[] =
	{
	{"-exit" , StrMatch, Exit},
	{"-remove", StrMatch, RemoveFile},
	{"-count", StrMatch, CountLines},
	{"<", AppendMatch, Append},
	{"", ReturnTrue, AppendLine} 
	};
	size_t cmd; 
	Status status = ERROR;

	for (cmd = 0; cmd < NUM_OF_CMD; cmd++)
	{
		if (command[cmd].IsMatch(command[cmd].cmd_id, line) == SUCCESS)
		{
			status = command[cmd].exec(file_name, line);
			break;
		}
	}
	
	return status;
}

/*********************************************************/

Status AppendLine(char* file_name, char* line)
{
	FILE* file;

	if (!(file = fopen(file_name, "a")) ||
		!fputs(line, file))
	{
		return ERROR;
	}

	fputc('\n', file);
	fclose(file);
	
	return SUCCESS;
}

/*********************************************************/

Status RemoveFile(char* file_name, char* line)
{
	(void) line;
	return -remove(file_name);
}

/*********************************************************/

Status CountLines(char* file_name, char* line)
{
	FILE* file;
	int count = 0;
	char c;
	(void) line;

	if (!(file = fopen(file_name, "r")))
	{
		return ERROR;
	}

	for (c = fgetc(file); c != EOF; c = fgetc(file))
	{
		if (c == '\n')
		{
			count++;
		}
	}

	printf("Line count is: %d\n", count);

	return -fclose(file);
}

/*********************************************************/

Status Exit(char* file_name, char* line)
{
	(void) file_name;
	(void) line;
	
	return EXIT;
}

/*********************************************************/

Status PreAppend(char* file_name, char* line)
{
	FILE* file = NULL;
	FILE* temp = NULL;
	char buffer[5];
	long size;

	file = fopen(file_name, "r");
	temp = fopen("temp_file", "w");

	if (!file || !temp)
	{
		return ERROR;
	}

	fputs(line, temp);

	while((size = fread(buffer, 1, sizeof(buffer), file)) > 0)
	{
		fwrite(buffer, 1, size, temp);
	}

	fclose(temp);
	fclose(file);
	remove(file_name);
	rename("temp_file", file_name);

	return SUCCESS;
}

/*********************************************************/

Status StrMatch(char* id, char* input)
{
	if (strncmp(id, input, strlen(id)) == SUCCESS)
	{
		return SUCCESS;
	}

	return ERROR; 
}

/*********************************************************/

Status AppendMatch(char *id, char* input)
{
	return !(*id == *input);
}

/*********************************************************/

Status ReturnTrue (char* id, char* input)
{
	(void) id;
	(void) input;
	
	return SUCCESS;
}

/*********************************************************/

Status Append(char* file_name, char* line)
{
	return PreAppend(file_name, line+1);
}

/*********************************************************/

void Verify(int status)
{
	if (status == ERROR || status == EXIT)
	{
		printf ("Error %d dedected\n", status);
		exit (1);
	}
}

/*********************************************************/






