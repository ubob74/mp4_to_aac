#ifndef _ADTS_H
#define _ADTS_H

#include <stdint.h>

#define ADTS_HDR_SIZE 7

struct adts_fixed_header {
	uint32_t syncword				:12;
	uint32_t id						:1;
	uint32_t layer					:1;
	uint32_t prot_abs				:1;
	uint32_t profile				:2;
	uint32_t sampling_freq_index	:4;
	uint32_t private_bit			:1;
	uint32_t channel_conf			:3;
	uint32_t orig_copy				:1;
	uint32_t home					:1;
} __attribute__ ((packed));

struct adts_var_header {
	uint32_t copyright_id_bit		:1;
	uint32_t copyright_id_start		:1;
	uint32_t frame_length			:13;
	uint32_t buffer_fullness		:11;
	uint32_t raw_data_blocks_in_frame	:2;
} __attribute__ ((packed));

struct adts_header {
	struct adts_fixed_header f_hdr;
	struct adts_var_header v_hdr;
};

struct adts_header *alloc_adts_header(void);
void release_adts_header(struct adts_header *hdr);

void adts_set_syncword(struct adts_fixed_header *f_hdr);
void adts_set_id(struct adts_fixed_header *f_hdr, int id);

void adts_header_dump(struct adts_header *hdr);

#endif
