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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stb.h>

/* stsd box */
int process_stsd_box_data(int fd, struct stsd *stsd)
{
	if (!stsd)
		return -1;

	return 0;
}

void dump_stsd_box_data(struct stsd *stsd)
{
	if (!stsd)
		return;
}

/* stsc box */
int process_stsc_box_data(int fd, struct stsc *stsc)
{
	uint32_t i, entry;
	uint32_t total_count = 0;

	if (!stsc)
		return -1;

	stsc->sample2chunk_table = NULL;
	stsc->count = swab32(*(uint32_t *)(BOX_DATA(stsc->box) + 4));
	stsc->sample2chunk_table = (uint32_t *)(BOX_DATA(stsc->box)) + 2;

	total_count = stsc->count * 3;

	for (i = 0; i < total_count; i++) {
		entry = swab32(stsc->sample2chunk_table[i]);
		stsc->sample2chunk_table[i] = entry;
	}

	return 0;
}

struct stsc_entry *get_stsc_entry(uint32_t i, struct stsc *stsc)
{
	struct stsc_entry *entry = NULL;

	if (!stsc)
		return NULL;

	if (i >= stsc->count) {
		fprintf(stderr, "count: %u\n", i);
		return NULL;
	}

	entry = (struct stsc_entry *)(stsc->sample2chunk_table + i * 3);

	return entry;
}

#ifdef DEBUG
void dump_stsc_box_data(struct stsc *stsc)
{
	uint32_t i;
	const char *s1 = "first chunk";
	const char *s2 = "samples per chunk";
	const char *s3 = "samples description index";
	struct stsc_entry *entry = NULL;

	if (!stsc)
		return;

	fprintf(stderr, "'stsc' entry count: %u\n", stsc->count);
	fprintf(stderr, "%-20s%-20s%-30s\n", s1, s2, s3);

	for (i = 0; i < stsc->count; i++) {
		if ((entry = get_stsc_entry(i, stsc)))
			fprintf(stderr, "%-20u%-20u%-30u\n", entry->first_chunk,
				entry->sample_per_chunk, entry->sample_desc_idx);
	}
}
#else
void dump_stsc_box_data(struct stsc *stsc) {}
#endif /* DEBUG */

/* stsz box */
int process_stsz_box_data(int fd, struct stsz *stsz)
{
	uint32_t i = 0;

	if (!stsz)
		return -1;

	stsz->sample_size_table = NULL;
	stsz->sample_size = swab32(*(uint32_t *)(BOX_DATA(stsz->box) + 4));
	stsz->sample_count = swab32(*(uint32_t *)(BOX_DATA(stsz->box) + 4 + 4));

	if (stsz->sample_size == 0) {
		stsz->sample_size_table = (uint32_t *)(BOX_DATA(stsz->box)) + 3;

		uint32_t entry_size = 0;
		for (i = 0; i < stsz->sample_count; i ++) {
			entry_size = swab32(stsz->sample_size_table[i]);
			stsz->sample_size_table[i] = entry_size;
		}
	}

	return 0;
}

uint32_t get_sample_size(uint32_t i, struct stsz *stsz)
{
	if (!stsz)
		return 0;

	if (i >= stsz->sample_count)
		return 0;

	return stsz->sample_size_table[i];
}

#ifdef DEBUG
void dump_stsz_box_data(struct stsz *stsz)
{
	uint32_t i;
	uint32_t entry_size = 0;

	if (!stsz)
		return;

	fprintf(stderr, "Sample size: %u\nSample count: %u\n",
			stsz->sample_size, stsz->sample_count);

	if (stsz->sample_size_table) {
		for (i = 0; i < stsz->sample_count; i++) {
			if (!(i%10))
				fprintf(stderr, "\n");
			entry_size = get_sample_size(i, stsz);
			fprintf(stderr, "%.8X ", entry_size);
		}
		fprintf(stderr, "\n");
	}
}
#else
void dump_stsz_box_data(struct stsz *stsz) {}
#endif /* DEBUG */

/* stco box */
int process_stco_box_data(int fd, struct stco *stco)
{
	uint32_t i;
	uint32_t entry;

    if (!stco)
        return -1;

    stco->entry_count = swab32(*(uint32_t *)(BOX_DATA(stco->box) + 4));
    stco->chunk_offset_table = (uint32_t *)(BOX_DATA(stco->box)) + 2;

	for (i = 0; i < stco->entry_count; i++) {
		entry = swab32(stco->chunk_offset_table[i]);
		stco->chunk_offset_table[i] = entry;
	}

	return 0;
}

uint32_t get_chunk_offset(uint32_t i, struct stco *stco)
{
	if (!stco)
		return 0;

	if (i >= stco->entry_count)
		return 0;

	return stco->chunk_offset_table[i];
}

#ifdef DEBUG
void dump_stco_box_data(struct stco *stco)
{
	uint32_t i;
	uint32_t chunk_offset;

	if (!stco)
		return;

	fprintf(stderr, "Chunk offset table entry count: %u\n", stco->entry_count);
	fprintf(stderr, "Chunk offset table:\n");

	for (i = 0; i < stco->entry_count; i++) {
		if (i && !(i%10))
			fprintf(stderr, "\n");

		chunk_offset = get_chunk_offset(i, stco);
		fprintf(stderr, "%.8X ", chunk_offset);
	}
	fprintf(stderr, "\n");
}
#else
void dump_stco_box_data(struct stco *stco) {}
#endif /* DEBUG */
