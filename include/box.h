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


#ifndef _BOX_H_
#define _BOX_H_

#include <stdint.h>

struct box_header
{
	uint32_t size;
	char type[4];
	uint64_t ext_size;
	uint8_t ver_flags;
};

struct box_data
{
	uint8_t *data;
	uint64_t offset;
	uint64_t size;
};

struct box
{
	int level;
	struct box_header hdr;
    struct box_data data;

    /* box offset */
    uint64_t offset;

    /* box header size */
    uint32_t hdr_size;

    /* box total size */
    uint64_t total_size;

    /* next box offset */
    uint64_t next_box_offset;

    /* parent box */
    struct box *parent;

    /* next box */
    struct box *next;

    /* previous box */
    struct box *prev;

    /* child box */
    struct box *child;
};

#define BOX_DATA(box) (box)->data.data

int get_box_list(int fd, struct box **head, struct box *parent);
struct box * find_box(struct box *head, const char *type);
int read_box_data(int f, struct box *);
void release_box_list(struct box **head);
void release_box(struct box *box);
void dump_box(struct box *box);

#endif /* _BOX_H_ */


