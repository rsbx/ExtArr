/*\
***  ExtArr.c: Extensible Arrays.
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


#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "ExtArr.h"


#ifndef QUIET_UNUSED
#define QUIET_UNUSED(var)		var = var
#endif /* QUIET_UNUSED */

#define ALIGN_UP(ADDR, ALIGN)		(((uintptr_t)(ADDR)+(ALIGN)-1) & ~((ALIGN)-1))

#define SIZE_BLOCK_DATA(EXTARR)		(ALIGN_UP((EXTARR)->elementsize*(1ul<<(EXTARR)->l2block), sizeof(ExtArr_Chunk_t *)))
#define SIZE_INDIRECT_DATA(EXTARR)	(sizeof(ExtArr_Chunk_t *)*(1ul<<(EXTARR)->l2indirect))


typedef union ExtArr_Chunk	ExtArr_Chunk_t;

union ExtArr_Chunk
	{
	ExtArr_Chunk_t *	chunk;
	void *			block;
	};

struct ExtArr
	{
	ExtArr_Chunk_t *	chunk_tree;
	ExtArr_Chunk_t *	chunk_list;
	ExtArr_Chunk_t *	block_list;
	unsigned short		levels;
	size_t			elementsize;
	unsigned short		l2block;
	unsigned short		l2indirect;
	int (*mem_alloc)(void *mem_data, size_t size, void **ptr);
	int (*mem_free)(void *mem_data, size_t size, void *ptr);
	void *			mem_data;
	};


ExtArr_t ExtArr_Create(
		size_t		elementsize,
		unsigned short	l2block,
		unsigned short	l2indirect,
		int (*mem_alloc)(void *mem_data, size_t size, void **ptr),
		int (*mem_free)(void *mem_data, size_t size, void *ptr),
		void *		mem_data
		)
	{
	ExtArr_t	extarr = NULL;

	if (mem_alloc(mem_data, sizeof(*extarr), (void **)&extarr))
		{
		return NULL;
		}

	extarr->chunk_tree = NULL;
	extarr->chunk_list = NULL;
	extarr->block_list = NULL;
	extarr->levels = 0;
	extarr->elementsize = elementsize;
	extarr->l2block = l2block;
	extarr->l2indirect = l2indirect;
	extarr->mem_alloc = mem_alloc;
	extarr->mem_free = mem_free;
	extarr->mem_data = mem_data;

	return extarr;
	}


int ExtArr_Destroy(
		ExtArr_t extarr
		)
	{
	int			error;
	ExtArr_Chunk_t *	chunk;
	ExtArr_Chunk_t *	chunk_next;

	chunk = extarr->chunk_list;
	while (chunk)
		{
		chunk_next = chunk[1ul<<extarr->l2indirect].chunk;
		if ((error = (extarr->mem_free)(
				extarr->mem_data,
				SIZE_INDIRECT_DATA(extarr) + sizeof(ExtArr_Chunk_t *),
				(void *)chunk
				)))
			{
			return error;
			}
		chunk = chunk_next;
		}

	chunk = extarr->block_list;
	while (chunk)
		{
		chunk_next = ((ExtArr_Chunk_t *)((char *)(chunk) + SIZE_BLOCK_DATA(extarr)))->block;
		if ((error = (extarr->mem_free)(
				extarr->mem_data,
				SIZE_BLOCK_DATA(extarr) + sizeof(ExtArr_Chunk_t *),
				(void *)chunk
				)))
			{
			return error;
			}
		chunk = chunk_next;
		}

	return (extarr->mem_free)(
			extarr->mem_data,
			sizeof(*extarr),
			(void *)extarr
			);
	}


static int new_block(
		ExtArr_t		extarr,
		ExtArr_Chunk_t **	blockp
		)
	{
	int	error;

	if ((error = (extarr->mem_alloc)(
			extarr->mem_data,
			SIZE_BLOCK_DATA(extarr) + sizeof(ExtArr_Chunk_t *),
			(void **)blockp
			)))
		{
		return error;
		}

	(void)memset(*blockp, 0, SIZE_BLOCK_DATA(extarr));

	((ExtArr_Chunk_t *)((char *)(*blockp) + SIZE_BLOCK_DATA(extarr)))->block = extarr->block_list;
	extarr->block_list = *blockp;

	return 0;
	}


static int new_indirect(
		ExtArr_t		extarr,
		ExtArr_Chunk_t **	chunkp
		)
	{
	int	error;

	if ((error = (extarr->mem_alloc)(
			extarr->mem_data,
			SIZE_INDIRECT_DATA(extarr) + sizeof(ExtArr_Chunk_t *),
			(void **)chunkp
			)))
		{
		return error;
		}

	(void)memset(*chunkp, 0, SIZE_INDIRECT_DATA(extarr));

	(*chunkp)[1ul<<extarr->l2indirect].chunk = extarr->chunk_list;
	extarr->chunk_list = *chunkp;

	return 0;
	}


void * ExtArr_Addr(
		ExtArr_t	extarr,
		ExtArr_index_t	index
		)
	{
	ExtArr_Chunk_t *	chunk;
	ExtArr_Chunk_t **	chunkp;
	unsigned int		i;
	ExtArr_index_t		block_index;

	/*
	 * Add new levels as needed.
	 */
	while (index > (1ul<<(extarr->levels*extarr->l2indirect+extarr->l2block))-1)
		{
		if (new_indirect(extarr, &chunk))
			{
			return NULL;
			}

		chunk[0].chunk = extarr->chunk_tree;
		extarr->chunk_tree = chunk;
		extarr->levels++;
		}

	/*
	 * Fill in indirect chunks as needed.
	 */
	block_index = index & ((1ul<<extarr->l2block)-1);
	index >>= extarr->l2block;

	chunkp = &(extarr->chunk_tree);
	for (i=extarr->levels; i; i--)
		{
		ExtArr_index_t	chunk_index = (index>>((i-1)*extarr->l2indirect)) & ((1ul<<extarr->l2indirect)-1);

		if (!*chunkp)
			{
			if (new_indirect(extarr, chunkp))
				{
				return NULL;
				}
			}

		chunkp = &(*chunkp)[chunk_index].chunk;
		}

	/*
	 * Add block if needed.
	 */
	if (!*chunkp)
		{
		if (new_block(extarr, chunkp))
			{
			return NULL;
			}
		}

	return (char *)(*chunkp) + extarr->elementsize * block_index;
	}
