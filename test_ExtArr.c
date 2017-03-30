/*\
***  test_ExtArr.c: Code to test Extensible Arrays library.
\*/

/*\
*#*  Copyright (c) 2017 Raymond S Brand
*#*  All rights reserved.
*#*
*#*  Redistribution and use in source and binary forms, with or without
*#*  modification, are permitted provided that the following conditions
*#*  are met:
*#*
*#*   * Redistributions of source code must retain the above copyright
*#*     notice, this list of conditions and the following disclaimer.
*#*
*#*   * Redistributions in binary form must reproduce the above copyright
*#*     notice, this list of conditions and the following disclaimer in
*#*     the documentation and/or other materials provided with the
*#*     distribution.
*#*
*#*   * Redistributions in source or binary form must carry prominent
*#*     notices of any modifications.
*#*
*#*   * Neither the name of the Raymond S Brand nor the names of its
*#*     contributors may be used to endorse or promote products derived
*#*     from this software without specific prior written permission.
*#*
*#*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*#*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*#*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*#*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*#*  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*#*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*#*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*#*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*#*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*#*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*#*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*#*  POSSIBILITY OF SUCH DAMAGE.
\*/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include "ExtArr.h"


#ifndef QUIET_UNUSED
#define QUIET_UNUSED(var)	var = var
#endif /* QUIET_UNUSED */


static int mem_alloc(void *mem_data, size_t size, void **ptr)
	{
	void *error = malloc(size);

	QUIET_UNUSED(mem_data);

	if (!error)
		{
		return size ? errno : EINVAL;
		}

	*ptr = error;
	return 0;
	}


static int mem_free(void *mem_data, size_t size, void *ptr)
	{
	QUIET_UNUSED(mem_data);
	QUIET_UNUSED(size);

	free(ptr);
	return 0;
	}


int main(int argc, const char **argv)
	{
	ExtArr_t	extarr;
	void *		data;
//	uintptr_t	i;
	uint32_t	i;

	QUIET_UNUSED(argc);
	QUIET_UNUSED(argv);

	if (!(extarr = ExtArr_Create(
//			sizeof(uintptr_t),
			sizeof(uint32_t),
			2,
			3,
			&mem_alloc,
			&mem_free,
			NULL
			)))
		{
		printf("Create error\n");
		exit(1);
		}

	for (i=1024*1024+2; i; i--)
//	for (i=0; i<1024+2; i++)
		{
//		if ((error = ExtArr_Addr(extarr, i, &data)))
		if (!(data = ExtArr_Addr(extarr, i)))
			{
			printf("Addr error\n");
			exit(1);
			}
//		*(uintptr_t *)data = i;
		*(uint32_t *)data = i;
		}

	for (i=0; i<1024*1024+8; i++)
		{
//		if ((error = ExtArr_Addr(extarr, i, &data)))
		if (!(data = ExtArr_Addr(extarr, i)))
			{
			printf("Addr error\n");
			exit(1);
			}
//		if (i != *(uintptr_t *)data)
		if (i != *(uint32_t *)data)
			{
//			printf("Data error: %lu != %lX\n", i, *(uintptr_t *)data);
			printf("Data error: %u != %X\n", i, *(uint32_t *)data);
			}
		}

	if (ExtArr_Destroy(extarr))
		{
		printf("Destroy error\n");
		exit(1);
		}

	exit(0);
	}
