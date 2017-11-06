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
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <trak.h>
#include <stb.h>
#include <utils.h>

static void release_trak_info(trak_info_t *trak_info)
{
    /* TODO */

    return;
}

static int get_trak_child_box_list(trak_info_t *trak_info, struct box *head)
{
    int i;
    struct box *tmp;
    trak_ctx_t *ctx = trak_info->ctx;
    const char *subbox_list[] = { "mdia", "minf", "stbl", NULL };

    for (i = 0; i < 4; i++) {
		get_box_list(ctx->fd, &head->child, head);

		if (subbox_list[i] == NULL) {
			trak_info->stbl = head;
			break;
		}

		for (tmp = head->child; (tmp); tmp = tmp->next) {
			fprintf(stderr, "%s\n", tmp->hdr.type);
            if (!strncmp(tmp->hdr.type, subbox_list[i], 4)) {
                fprintf(stderr, "'%s' box has been found\n", subbox_list[i]);
                break;
            }
        }

        if (!tmp)
            return -1;

        head = tmp;
    }

    return 0;
}

static int get_trak_sample_table_box(trak_info_t *trak_info)
{
    int ret = -1;
    trak_ctx_t *ctx;

    if (!trak_info)
        goto err;

    if (!(ctx = trak_info->ctx))
        goto err;

    trak_info->stb.stsd.box = find_box(trak_info->stbl, "stsd");
    trak_info->stb.stsc.box = find_box(trak_info->stbl, "stsc");
    trak_info->stb.stsz.box = find_box(trak_info->stbl, "stsz");
    trak_info->stb.stco.box = find_box(trak_info->stbl, "stco");

    if (!trak_info->stb.stsd.box || !trak_info->stb.stsc.box ||
        !trak_info->stb.stsz.box || !trak_info->stb.stco.box)
        goto err;

    if (read_box_data(ctx->fd, trak_info->stb.stsd.box) < 0)
        goto err;

    if (read_box_data(ctx->fd, trak_info->stb.stsc.box) < 0)
        goto err;

    if (read_box_data(ctx->fd, trak_info->stb.stsz.box) < 0)
        goto err;

    if (read_box_data(ctx->fd, trak_info->stb.stco.box) < 0)
        goto err;

    ret = 0;

err:
    return ret;
}

trak_ctx_t *init_trak_ctx(const char *filename)
{
    trak_ctx_t *ctx = NULL;

    if (!(filename))
        goto err;

    if (!(ctx = (trak_ctx_t *)calloc(1, sizeof (trak_ctx_t))))
        goto err;

    memset(ctx, 0, sizeof(trak_ctx_t));

    ctx->fd = -1;
    ctx->nr_trak_num = 0;
    ctx->trak_info = NULL;
    ctx->head = NULL; /* box list head */

    if ((ctx->fd = open(filename, O_RDWR)) < 0) {
        free(ctx);
        ctx = NULL;
    }

err:
    return ctx;
}

unsigned int read_trak_info(trak_ctx_t *ctx)
{
    struct box *head = NULL, *tmp = NULL;
    trak_info_t *trak_info = NULL, **tmp1 = NULL;

    if (!ctx)
        return;

    get_box_list(ctx->fd, &ctx->head, NULL);

    head = ctx->head;

    /* Find out 'moov' box */
    for (tmp = head; (tmp); tmp = tmp->next) {
        if (!strncmp(tmp->hdr.type, "moov", 4)) {
			fprintf(stderr, "Moov box has been found\n");
			get_box_list(ctx->fd, &tmp->child, tmp);
			break;
        }
    }

    /* Fault - return nothing */
    if (!tmp)
        return 0;

    /* Gathering of trak box info */
    for (tmp = tmp->child;(tmp);tmp = tmp->next) {
        if (!strncmp(tmp->hdr.type, "trak", 4)) {
            fprintf(stderr, "Trak box has been found\n");

            if (!(trak_info = (trak_info_t *)calloc(1, sizeof(trak_info_t))))
                return 0;

            memset(trak_info, 0, sizeof(trak_info));

            trak_info->ctx = ctx;
            trak_info->next = NULL;
            trak_info->trak = tmp;

            for (tmp1 = &ctx->trak_info; (*tmp1); tmp1 = &(*tmp1)->next);
            *tmp1 = trak_info;

            if (get_trak_child_box_list(trak_info, trak_info->trak) < 0)
				return 0;

            if (get_trak_sample_table_box(trak_info) < 0)
                return 0;

            if (process_stsz_box_data(ctx->fd, &trak_info->stb.stsz) < 0)
                return 0;

            if (process_stsc_box_data(ctx->fd, &trak_info->stb.stsc) < 0)
                return 0;

            if (process_stco_box_data(ctx->fd, &trak_info->stb.stco) < 0)
                return 0;

            ctx->nr_trak_num++;
        }
    }

    return ctx->nr_trak_num;
}

uint32_t read_data(trak_info_t *trak_info, uint8_t **data, uint32_t size, uint64_t offset)
{
	int fd;
	ssize_t retval = 0;

    if (!trak_info)
		goto err;

	if (!size || !offset)
		goto err;

	fd = trak_info->ctx->fd;

	*data = (uint8_t *)calloc(1, size);
	if (!*data)
		goto err;

	memset(*data, 0, size);
	lseek64(fd, offset, SEEK_SET);
	if ((retval = read(fd, *data, size)) <= 0) {
		perror("read");
		free(*data);
		*data = NULL;
	}

err:
	return (size_t)retval;
}

unsigned int get_trak_num(trak_ctx_t *ctx)
{
	return (ctx) ? ctx->nr_trak_num : 0;
}

trak_info_t * get_trak_info(trak_ctx_t *ctx, unsigned int trak_num)
{
	int i = 1;
	trak_info_t *current;

	if (!ctx)
		return NULL;

	if (trak_num > ctx->nr_trak_num)
		return NULL;

	for (current = ctx->trak_info; (current); current = current->next) {
		if (i++ == trak_num)
			break;
	}

	return current;
}

uint32_t get_trak_sample_count(trak_info_t *trak_info)
{
	struct stsz *stsz = NULL;

	if (!trak_info)
		return 0;

	stsz = &trak_info->stb.stsz;
	return stsz->sample_count;
}

uint32_t get_trak_sample_size(uint32_t i, trak_info_t *trak_info)
{
	uint32_t sample_count = 0;

	if (!trak_info)
		return 0;

	return get_sample_size(i, &trak_info->stb.stsz);
}

uint32_t get_trak_sample2chunk_count(trak_info_t *trak_info)
{
	struct stsc *stsc = &trak_info->stb.stsc;
    return stsc->count;
}

struct stsc_entry *get_trak_sample2chunk_table_entry(uint32_t i, trak_info_t *trak_info)
{
    struct stsc_entry *entry = NULL;

    if (!trak_info)
        return NULL;

    return get_stsc_entry(i, &trak_info->stb.stsc);
}

uint32_t get_trak_chunk_offset(uint32_t i, trak_info_t *trak_info)
{
    if (!trak_info)
        return 0;

    return get_chunk_offset(i, &trak_info->stb.stco);
}

void release_trak_ctx(trak_ctx_t *ctx)
{
	int i = 0;
	trak_info_t *current = NULL, *tmp = NULL;

	if (!ctx)
		return;

	if (ctx->fd) {
		close(ctx->fd);
		ctx->fd = -1;
	}

	current = ctx->trak_info;
	while ((current)) {
		tmp = current->next;
		release_trak_info(current);
		free(current);
		current = tmp;
	}

	release_box_list(&ctx->head);
	free(ctx);
}

#ifdef DEBUG
void dump_trak_info(trak_info_t *trak_info)
{
    trak_ctx_t *ctx = NULL;
    struct box *head;

    if (!trak_info)
        return;

    if (!(ctx = trak_info->ctx))
        return;

    if ((head = trak_info->trak->child))
        dump_box(head);

    dump_stsz_box_data(&trak_info->stb.stsz);
    dump_stsc_box_data(&trak_info->stb.stsc);
    dump_stco_box_data(&trak_info->stb.stco);
}
#else
void dump_trak_info(trak_info_t *trak_info) {}
#endif /* DEBUG */
