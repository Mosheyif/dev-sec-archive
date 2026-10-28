/*********************************************************
*
* Description: Declarations for struct and union exercises
* Author: Moshe Yifrah
*
*********************************************************/

#ifndef __STRUCT_H__
#define __STRUCT_H__

/* Includes ******************************************************/

#include <stddef.h> /* size_t */

/* Macros *******************************************************/

#define NUM_OF_TYPES 3
#define MAX2(a,b) (((a)>(b))?(a):(b))
#define MAX3(a,b,c) ( ((a)>(b)) ? (((a)>(c))?(a):(c)) : (((b)>(c))?(b):(c)) )
#define SIZEOF_VAR(x) ( (char*)(&(x) + 1) - (char*)(&(x)) )
#define SIZEOF_TYPE(type) ( (size_t)( ((type*)0) + 1 ) )

/* Declarations ************************************************/

typedef union
{

  int integer;
  float floating_p;
  char* string;
  
} Types;

typedef struct Elements elements;

typedef int (*add_func)(elements*, int);
typedef void (*print_func)(const elements*);
typedef int (*cleanup)(elements*);

struct Elements 
{
  Types types;
  add_func AddFunc;
  print_func PrintFunc;
  cleanup CleanUp;
  
};

enum _Types
{
  INT,
  FLOAT,
  STRING
};

/* Implemention Functions ***********************************/

void ManageStruct ();
int InitArr(elements []);
int InitInt(elements*, int);
int InitFloat(elements*, float);
int InitString(elements*, char*);
int AddInt(elements*, int);
int AddFloat(elements*, int);
int AddString(elements*, int);
void PrintInt(const elements*);
void PrintFloat(const elements*);
void PrintString(const elements*);
int CleanUpInt(elements*);
int CleanUpFloat(elements*);
int CleanUpString(elements*);
void PrintArr(const elements []);
int AddAll(elements []);
int CleanArr(elements []);

/* Helping Functions ***************************************/

size_t StrLen(const char*);
char* StrNCpy(char*, const char*, size_t);
char* StrCat(char*, const char*);
int GetIntSize(int);
char* ConvertIntToString(int, int);

/* Testing Functions **************************************/

void print_result(char*, int);

#endif



















