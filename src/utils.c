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

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

uint32_t swab32(uint32_t x)
{
	uint32_t y = 0;

	*((uint8_t *)&y + 3) = *(uint8_t *)&x;
	*((uint8_t *)&y + 2) = *((uint8_t *)&x + 1);
	*((uint8_t *)&y + 1) = *((uint8_t *)&x + 2);
	*(uint8_t *)&y = *((uint8_t *)&x + 3);

	return y;
}

uint64_t swab64(uint64_t x)
{
	uint64_t y = 0;

	*((uint8_t *)&y + 7) = *(uint8_t *)&x;
	*((uint8_t *)&y + 6) = *((uint8_t *)&x + 1);
	*((uint8_t *)&y + 5) = *((uint8_t *)&x + 2);
	*((uint8_t *)&y + 4) = *((uint8_t *)&x + 3);
	*((uint8_t *)&y + 3) = *((uint8_t *)&x + 4);
	*((uint8_t *)&y + 2) = *((uint8_t *)&x + 5);
	*((uint8_t *)&y + 1) = *((uint8_t *)&x + 6);
	*(uint8_t *)&y = *((uint8_t *)&x + 7);

	return y;
}
