/*********************************************************
 *
 * Description: Implicit linking demo — calls PrintGlobalVariableAddress,
 *              Foo and Foo2 from two separate shared libraries
 * Author: Moshe Yifrah
 *
 * Compile with: gcc -o immain immain.c -L. -ldlib1 -ldlib2 -ldefs
 *
 * Run by: LD_LIBRARY_PATH=. ./immain
 *
 *********************************************************/

void PrintGlobalVariableAddress();
void Foo();
void Foo2();

int main()
{
	PrintGlobalVariableAddress();
	Foo();
	Foo2();

	return 0;
}
