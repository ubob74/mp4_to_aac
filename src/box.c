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

#include <trak.h>
#include <box.h>
#include <utils.h>

static struct box *init_box(int level, uint64_t box_offset)
{
	struct box *box = NULL;

	if (!(box = (struct box *)calloc(1, sizeof(struct box))))
		return NULL;

	memset(box, 0, sizeof(struct box));
	box->data.data = NULL;
	box->data.offset = 0;
	box->data.size = 0;
	box->level = level;
	box->offset = box_offset;

	return box;
}

static struct box *get_box(int fd, uint64_t box_offset, int level)
{
    /*int n = 0;*/
    uint8_t buf[36];
    ssize_t ret = 0;
    uint32_t tmp_size32 = 0;
    uint64_t tmp_size64 = 0;
    struct box *box = NULL;

    memset(buf, 0, sizeof(buf));
 
    lseek64(fd, box_offset, SEEK_SET);
	ret = read(fd, buf, sizeof(buf));
	if (ret != sizeof(buf))
		return NULL;

    box = init_box(level, box_offset);
	if (!box)
        return NULL;

    tmp_size32 = *(uint32_t *)buf;
    box->hdr.size = swab32(tmp_size32);
    memcpy(box->hdr.type, buf + 4, 4);

    box->total_size = box->hdr.size;

	if (box->hdr.size == 1) {
		tmp_size64 = *(uint64_t *)(buf + 8);
		/*for (n = 0; n < 8; n++)
			*((uint8_t *)&box->hdr.ext_size + n) = *((uint8_t *)&tmp_size64 + 7 - n);*/
		box->hdr.ext_size = swab64(tmp_size64);
        box->total_size = box->hdr.ext_size;
    }

    box->next_box_offset = box->offset + box->total_size;
    box->data.offset = box->offset + 8;

    if (box->hdr.ext_size != 0)
        box->data.offset += 8;

    box->data.size = box->next_box_offset - box->data.offset;

    return box;
}

int get_box_list(int fd, struct box **head, struct box *parent)
{
	int level = 0;
	struct box *box = NULL, **current;
	struct box *prev = NULL;
	uint64_t box_offset = 0, border = 0;

	if (parent) {
		level = parent->level + 1;
		box_offset = parent->data.offset;
		border = parent->next_box_offset;
	}

	for (current = head;;*current = box, current = &box->next) {
		if (!(box = get_box(fd, box_offset, level)))
			break;

		box->prev = prev;
		prev = box;

		if (parent) {
			if (box_offset == border)
				break;
			box->parent = parent;
			box_offset += box->total_size;
		} else {
			box_offset = box->next_box_offset;
		}
	}

	return 0;
}

struct box *find_box(struct box *head, const char *type)
{
	struct box *tmp = NULL, *current = NULL;

	if (!head || !type)
		return NULL;

	for (current = head; (current); current = current->next) {
		if (!strncmp(current->hdr.type, type, 4))
			return current;

		if ((tmp = find_box(current->child, type)))
			return tmp;
	}

	return NULL;
}

int read_box_data(int fd, struct box *box)
{
	int ret = -1;

	if ((fd < 0) || !box)
		goto err;

	box->data.data = (uint8_t *)malloc(box->data.size);
	if (!box->data.data)
		goto err;

	ret = 0;
	memset(box->data.data, 0, box->data.size);
	lseek64(fd, box->data.offset, SEEK_SET);
	if (read(fd, (void *)box->data.data, box->data.size) <= 0) {
		free(box->data.data);
		box->data.data = NULL;
		ret = -1;
	}

err:
	return ret;
}

void release_box_list(struct box **head)
{
    struct box *tmp = NULL, *current = NULL;

	current = *head;
	while ((current)) {
		tmp = current->next;
		if (current->child)
			release_box_list(&current->child);
		release_box(current);
		free(current);
		current = tmp;
	}
}

void release_box(struct box *box)
{
    if (!box)
        return;

	/* Release box data */
	if (box->data.data) {
		free(box->data.data);
		box->data.data = NULL;
	}

	memset(box, 0, sizeof(struct box));
}

void dump_box(struct box *box)
{
	int i;
	struct box *current;

	if (!box)
		return;

	for (current = box; (current); current = current->next) {
		for (i = 0; i < box->level; i++)
			fprintf(stderr, "\t");
		fprintf(stderr, "%s\n", current->hdr.type);
		if (current->child)
			dump_box(current->child);
	}
}
