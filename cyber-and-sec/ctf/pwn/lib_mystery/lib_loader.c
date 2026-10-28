/*********************************************************
*
* Description: Loads libLibMistery.so via dlopen, resolves its
*              "strcpy" with dlsym, and calls it to reveal the
*              library's overridden behaviour
* Author: Moshe Yifrah
*
* Compile with: gcc -o lib_loader lib_loader.c -ldl
*
* Run by: ./lib_loader
*
*********************************************************/

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (*strcpy_fn)(char *, const char *);

int main()
{
	void *handle = dlopen("./libLibMistery.so", RTLD_LAZY);
	if (!handle)
	{
		fprintf(stderr, "dlopen failed: %s\n", dlerror());
		return 1;
	}

	dlerror();

	strcpy_fn lib_strcpy = (strcpy_fn)dlsym(handle, "strcpy");
	char *err = dlerror();
	if (err)
	{
		fprintf(stderr, "dlsym failed: %s\n", err);
		dlclose(handle);
		return 1;
	}

	char src[] = "12345678901234567890123456789012345678901234567890";
	char dst[64];

	lib_strcpy(dst, src);
	puts(dst);

	if (dlclose(handle) != 0)
	{
		fprintf(stderr, "dlclose failed: %s\n", dlerror());
		return 1;
	}

	return 0;
}
