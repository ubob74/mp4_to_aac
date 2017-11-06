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


#ifndef _STB_H_
#define _STB_H_

#include <box.h>

struct stsd {
	struct box *box;
};

/* Sample-To-Chunk box */
struct stsc {
	struct box *box;
	uint32_t count;
	uint32_t *sample2chunk_table;
};

struct stsc_entry {
	uint32_t first_chunk;
	uint32_t sample_per_chunk;
	uint32_t sample_desc_idx;
} __attribute__ ((packed));

/* Sample Size box */
struct stsz
{
	struct box *box;
	uint32_t sample_count;
	uint32_t sample_size;
	uint32_t *sample_size_table;
};

/* Chunk Offset Box */
struct stco {
    struct box *box;
    uint32_t entry_count;
    uint32_t *chunk_offset_table;
};

/* Sample table box */
struct stb {
	struct stsd stsd; /* sample description */
	struct stsc stsc; /* sample-to-chunk */
	struct stsz stsz; /* sample sizes */
	struct stco stco; /* chunk offset */
};

int process_stsd_box_data(int, struct stsd *);
int process_stsc_box_data(int, struct stsc *);
int process_stsz_box_data(int, struct stsz *);
int process_stco_box_data(int, struct stco *);

uint32_t get_sample_size(uint32_t, struct stsz *);
struct stsc_entry *get_stsc_entry(uint32_t, struct stsc *);
uint32_t get_chunk_offset(uint32_t, struct stco *);

void dump_stsd_box_data(struct stsd *);
void dump_stsc_box_data(struct stsc *);
void dump_stsz_box_data(struct stsz *);
void dump_stco_box_data(struct stco *);

#endif /* _STB_H_ */
