/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/********************************************************************
* Author: Vladimir Meshkov, <ubob@mail.ru>
*
* Copyright (c) 2012,  All rights reserved.
*
********************************************************************/


#ifndef _TRAK_H_
#define _TRAK_H_

#include <stb.h>

struct trak_info;

typedef struct trak_ctx {
    int fd;
    unsigned int nr_trak_num;
    struct box *head;
    struct trak_info *trak_info;
} trak_ctx_t;

typedef struct trak_info {
    int num;
    trak_ctx_t *ctx;
    struct box *trak;
    struct box *stbl;
    struct stb stb;
    struct trak_info *next;
} trak_info_t;

trak_ctx_t * init_trak_ctx(const char *);
unsigned int read_trak_info(trak_ctx_t *);
uint32_t read_data(trak_info_t *trak_info, uint8_t **data, uint32_t size, uint64_t offset);
unsigned int get_trak_num(trak_ctx_t *);
trak_info_t * get_trak_info(trak_ctx_t *, unsigned int);
void dump_trak_info(trak_info_t *);
void release_trak_ctx(trak_ctx_t *);
uint32_t get_trak_sample_count(trak_info_t *);
uint32_t get_trak_sample2chunk_count(trak_info_t *);
struct stsc_entry *get_trak_sample2chunk_table_entry(uint32_t, trak_info_t *);
uint32_t get_trak_chunk_offset(uint32_t, trak_info_t *);

#endif /* _TRAK_H_ */


