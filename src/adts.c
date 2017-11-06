#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <adts.h>

static void adts_fixed_header_dump(struct adts_fixed_header *f_hdr)
{
}

static void adts_var_header_dump(struct adts_var_header *v_hdr)
{
}

void adts_header_dump(struct adts_header *hdr)
{
	adts_fixed_header_dump(&hdr->f_hdr);
	adts_var_header_dump(&hdr->v_hdr);
}

struct adts_header *alloc_adts_header(void)
{
	struct adts_header *adts;

	adts = (struct adts_header *)calloc(1, sizeof(*adts));
	if (!adts)
		return NULL;

	memset(&adts->f_hdr, 0, sizeof(struct adts_fixed_header));
	memset(&adts->v_hdr, 0, sizeof(struct adts_var_header));
	return adts;
}

void release_adts_header(struct adts_header *adts)
{
	if (adts == NULL)
		return;

	memset(&adts->f_hdr, 0, sizeof(struct adts_fixed_header));
	memset(&adts->v_hdr, 0, sizeof(struct adts_var_header));
	free(adts);
}

void adts_set_syncword(struct adts_fixed_header *f_hdr)
{
	f_hdr->syncword = 0xFFF;
}

void adts_set_id(struct adts_fixed_header *f_hdr, int id)
{
	f_hdr->id = id & 0x1;
}


