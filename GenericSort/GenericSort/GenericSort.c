#include <stdio.h>
#include <stdlib.h>

#define ELEMENT_AT(arr, i, w) (((char*)arr) + ((i)*(w)))

void sort(void * arr, const size_t num_elements, const size_t size, int(*compare)(const void* a, const void* b));
int charCompare(const void * a, const void * b) { return (*(char *)a - *(char *)b); }
int intCompare(const void * a, const void * b) { return (*(int *)a - *(int *)b); }


/**
*
*	Selection Sort Test
*
*
*	Example usage
*	sort((void*)w, 5, sizeof(char), charCompare);
*
*/
void sort(void * arr, const size_t num_elements, const size_t size, int(*compare)(const void* a, const void* b))
{


	void* tempBuf = malloc(size);
	if (tempBuf == NULL)
		return NULL;


	for (size_t i = 0; i < num_elements - 1; i++)
	{
		for (size_t j = i + 1; j < num_elements; j++)
		{
			if (compare(ELEMENT_AT(arr, j, size), ELEMENT_AT(arr, i, size)) < 0) {
				memcpy(tempBuf, ELEMENT_AT(arr, j, size), size);
				memcpy(ELEMENT_AT(arr, j, size), ELEMENT_AT(arr, i, size), size);
				memcpy(ELEMENT_AT(arr, i, size), tempBuf, size);
			}

		}
	}

	free(tempBuf);

}
