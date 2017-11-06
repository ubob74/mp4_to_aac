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
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <trak.h>
#include <stb.h>
#include <adts.h>

static int fd;

static size_t read_samples(trak_info_t *trak_info, uint32_t nr_chunk,
					uint32_t sample_per_chunk, uint32_t start_sample);

static struct adts_header *get_adts_header(uint8_t id)
{
	struct adts_header *adts;
	struct adts_fixed_header *f_hdr;
	struct adts_var_header *v_hdr;
	uint32_t adts_hdr_sample_size = 0;
	const uint8_t adts_hdr_template[ADTS_HDR_SIZE] = { 0xFF, 0xF1, 0x50, 0x00, 0x00, 0x1F, 0xFC };

	adts = alloc_adts_header();
	if (!adts)
		return NULL;
	f_hdr = &adts->f_hdr;
	v_hdr = &adts->v_hdr;

	adts_set_syncword(f_hdr);
	adts_set_id(f_hdr, 0);

	adts_header_dump(adts);

	return adts;
}

static size_t read_samples(trak_info_t *trak_info, uint32_t nr_chunk, uint32_t sample_per_chunk, uint32_t start_sample)
{
    uint8_t id;
    uint32_t i;
    uint64_t chunk_offset = 0;
    uint64_t sample_offset = 0;
    uint32_t sample_size = 0, total_sample_size_in_chunk = 0, retval = 0;
	uint32_t adts_hdr_sample_size = 0;
    uint8_t *data = NULL;
	const uint8_t adts_hdr_template[ADTS_HDR_SIZE] = { 0xFF, 0xF1, 0x50, 0x00, 0x00, 0x1F, 0xFC };
	uint8_t adts_hdr[ADTS_HDR_SIZE];
	struct adts_header *adts;

	adts = get_adts_header(0);
	if (adts)
		release_adts_header(adts);

    chunk_offset = (uint64_t)get_trak_chunk_offset(nr_chunk - 1, trak_info);

    fprintf(stderr, "Chunk: %-7u\tstart sample: %-7u\tsample per chunk: %-7u\ttotal samples: %-7u\tchunk offset: %.8lX\n", \
            nr_chunk, start_sample, sample_per_chunk, (start_sample + sample_per_chunk), chunk_offset);

    sample_offset = chunk_offset;
    for (i = 0; i < sample_per_chunk; i++)
    {
        sample_size = (size_t)get_trak_sample_size(start_sample + i, trak_info);
        total_sample_size_in_chunk += sample_size;

        retval = read_data(trak_info, &data, sample_size, sample_offset);
		if (retval == sample_size) {
            id = data[0] >> 5;
			fprintf(stderr, "data[0][1]=%.2x %.2x\n", data[0], data[1]);
			if (id > 8)
                continue;

            memcpy(adts_hdr, adts_hdr_template, ADTS_HDR_SIZE);

            /* Check stereo/mono */
			switch (id) {
				case 0x0: /* single channel element (SCE) */
					adts_hdr[3] |= 0x40;
					break;
				case 0x1: /* channel pair element (CPE) */
					adts_hdr[3] |= 0x80;
					/* falling through */
				default:
					break;
            }

            adts_hdr_sample_size = sample_size + ADTS_HDR_SIZE;
            adts_hdr_sample_size &= 0x1FFF;
            adts_hdr_sample_size <<= 5;

            adts_hdr[5] |= *(uint8_t *)&adts_hdr_sample_size;
            adts_hdr[4] |= *((uint8_t *)&adts_hdr_sample_size + 1);
            adts_hdr[3] |= *((uint8_t *)&adts_hdr_sample_size + 2);

            if (write(fd, (void *)adts_hdr, ADTS_HDR_SIZE) < 0) {
                perror("write");
                free(data);
                break;
            }

            if (write(fd, data, retval) < 0) {
                perror("write");
                free(data);
                break;
            }

            free(data);

            sample_offset += retval;
        }
 
    }

    return retval;
}

int main(int argc, char **argv)
{
    unsigned int i = 0, n = 0;
    unsigned int total_samples = 0;
    unsigned int current_sample_per_chunk = 0;
    unsigned int nr_trak_num = 0;
    uint32_t delta = 0;
    uint32_t sample_count = 0;
    uint32_t sample2chunk_count = 0;
    uint32_t current_start_sample = 0;
    trak_ctx_t *ctx = NULL;
    trak_info_t *trak_info = NULL, *audio_trak_info = NULL;

    /* Sample-to-chunk entry */
	struct stsc_entry *current = NULL, *next = NULL;

	if (argc != 2) {
        fprintf(stderr, "Usage: %s <mp4 file>\n", argv[0]);
        return 0;
	}

    fd = -1;

    if (!(ctx = init_trak_ctx(argv[1])))
        return -1;

	nr_trak_num = read_trak_info(ctx);
	if (nr_trak_num == 0)
		goto err;

    fprintf(stderr, "Total trak number: %d\n", nr_trak_num);

    for (i = 1; i <= nr_trak_num; i++) {
        if (!(trak_info = get_trak_info(ctx, i)))
            break;

        fprintf(stderr, "Trak number: %d\n", i);
        dump_trak_info(trak_info);

        if (find_box(trak_info->trak, "smhd"))
            audio_trak_info = trak_info;
    }

    if (!audio_trak_info) {
        fprintf(stderr, "No audio trak is found\n");
        goto err;
    }

    if (!(sample2chunk_count = get_trak_sample2chunk_count(audio_trak_info)))
        goto err;

    if (!(sample_count = get_trak_sample_count(audio_trak_info)))
        goto err;

    fprintf(stderr, "stsc entry count: %d\n", sample2chunk_count);

    if ((fd = open("audio_trak.aac", O_CREAT|O_TRUNC|O_RDWR, 0644)) < 0) {
        perror("open");
        goto err;
    }

    for (i = 1, n = 0; n < sample2chunk_count; n++) {
        if (!(current = get_trak_sample2chunk_table_entry(n, audio_trak_info)))
            goto err;

        if (!(next = get_trak_sample2chunk_table_entry(n + 1, audio_trak_info))) {
            delta = (uint32_t)(sample_count - total_samples);
            fprintf(stderr, "Delta: %u\n", delta);
            break;
        }

        fprintf(stderr, "Next stsc entry first chunk: %u\n", next->first_chunk);

        for (; i < next->first_chunk; i++) {
            current_sample_per_chunk = current->sample_per_chunk;
            read_samples(audio_trak_info, i, current_sample_per_chunk, current_start_sample);
            current_start_sample += current_sample_per_chunk;
            total_samples = current_start_sample;
        }
    }

	current_sample_per_chunk = current->sample_per_chunk;
	read_samples(audio_trak_info, i, current_sample_per_chunk, current_start_sample);
	total_samples += current_sample_per_chunk;
	fprintf(stderr, "Total samples: %u\n", total_samples);

err:
    if (fd > 0)
        close(fd);

    release_trak_ctx(ctx);

    return 0;
}





