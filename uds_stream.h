#ifndef __UDS_STREAM_H__
#define __UDS_STREAM_H__

#include <stdint.h>

struct uds_stream {
	uint32_t cap;
	uint32_t len;
	uint8_t *curr;
	uint8_t *start;
};

typedef struct uds_stream uds_stream_t;

void uds_stream_init(uds_stream_t *uds_stream, unsigned char *data, unsigned int len);

void uds_stream_reset(uds_stream_t *uds_stream);

uint32_t uds_stream_left_len(uds_stream_t *uds_stream);

uint32_t uds_stream_len(uds_stream_t *uds_stream);

uint32_t uds_stream_cap(uds_stream_t *uds_stream);

uint8_t *uds_stream_ptr(uds_stream_t *uds_stream);

uint8_t *uds_stream_start_ptr(uds_stream_t *uds_stream);

uint32_t uds_stream_forward(uds_stream_t *uds_stream, uint32_t step);

uint32_t uds_stream_backward(uds_stream_t *uds_stream, uint32_t step);

uint32_t uds_stream_write_byte(uds_stream_t *uds_stream, uint8_t val);

uint32_t uds_stream_write_be16(uds_stream_t *uds_stream, uint16_t val);

uint32_t uds_stream_write_be32(uds_stream_t *uds_stream, uint32_t val);

uint32_t uds_stream_write_be64(uds_stream_t *uds_stream, uint64_t val);

uint32_t uds_stream_write_le16(uds_stream_t *uds_stream, uint16_t val);

uint32_t uds_stream_write_le32(uds_stream_t *uds_stream, uint32_t val);

uint32_t uds_stream_write_le64(uds_stream_t *uds_stream, uint64_t val);

uint32_t uds_stream_write_data(uds_stream_t *uds_stream, uint8_t *data, uint32_t len);

uint32_t uds_stream_write_string(uds_stream_t *uds_stream, const char *str);

uint8_t uds_stream_read_byte(uds_stream_t *uds_stream);

uint16_t uds_stream_read_be16(uds_stream_t *uds_stream);

uint32_t uds_stream_read_be32(uds_stream_t *uds_stream);

uint64_t uds_stream_read_be64(uds_stream_t *uds_stream);

uint16_t uds_stream_read_le16(uds_stream_t *uds_stream);

uint32_t uds_stream_read_le32(uds_stream_t *uds_stream);

uint64_t uds_stream_read_le64(uds_stream_t *uds_stream);

uint32_t uds_stream_read_data(uds_stream_t *uds_stream, uint8_t *buffer, uint32_t len);

#endif
