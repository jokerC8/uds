#include "uds_stream.h"
#include <string.h>

void uds_stream_init(uds_stream_t *uds_stream, unsigned char *data, unsigned int len)
{
	uds_stream->len = 0;
	uds_stream->cap = len;
	uds_stream->curr = data;
	uds_stream->start = data;
}

void uds_stream_reset(uds_stream_t *uds_stream)
{
	if (uds_stream) {
		uds_stream->curr = uds_stream->start;
		uds_stream->len = 0;
	}
}

unsigned int uds_stream_left_len(uds_stream_t *uds_stream)
{
	if (uds_stream->cap >= uds_stream->len) {
		return uds_stream->cap - uds_stream->len;
	}
	return 0;
}

unsigned int uds_stream_len(uds_stream_t *uds_stream)
{
	return uds_stream->len;
}

unsigned int uds_stream_cap(uds_stream_t *uds_stream)
{
	return uds_stream->cap;
}

unsigned char *uds_stream_ptr(uds_stream_t *uds_stream)
{
	return uds_stream->curr;
}

uint8_t *uds_stream_start_ptr(uds_stream_t *uds_stream)
{
	return uds_stream->start;
}

uint32_t uds_stream_forward(uds_stream_t *uds_stream, uint32_t step)
{
	uint32_t offset = (uds_stream->cap - uds_stream->len >= step) ? step : uds_stream->cap - uds_stream->len;

	uds_stream->curr += offset;
	uds_stream->len += offset;

	return offset;
}

uint32_t uds_stream_move_backward(uds_stream_t *uds_stream, uint32_t step)
{
	uint32_t offset = (uds_stream->len >= step) ? step : uds_stream->len;

	uds_stream->curr -= offset;
	uds_stream->len -= offset;

	return offset;
}

uint32_t uds_stream_write_byte(uds_stream_t *uds_stream, uint8_t val)
{
	if (uds_stream_left_len(uds_stream) >= 1) {
		*uds_stream->curr++ = val;
		++uds_stream->len;
		return 1;
	}
	return 0;
}

uint32_t uds_stream_write_be16(uds_stream_t *uds_stream, uint16_t val)
{
	uint32_t cnt = 0;

	cnt += uds_stream_write_byte(uds_stream, (uint8_t)(val >> 8));
	cnt += uds_stream_write_byte(uds_stream, (uint8_t)val);
	return cnt;
}

uint32_t uds_stream_write_be32(uds_stream_t *uds_stream, uint32_t val)
{
	uint32_t cnt = 0;

	cnt += uds_stream_write_be16(uds_stream, (uint16_t)(val >> 16));
	cnt += uds_stream_write_be16(uds_stream, (uint16_t)val);
	return cnt;
}

uint32_t uds_stream_write_be64(uds_stream_t *uds_stream, uint64_t val)
{
	uint32_t cnt = 0;

	cnt += uds_stream_write_be32(uds_stream, (uint32_t)(val >> 32));
	cnt += uds_stream_write_be32(uds_stream, (uint32_t)val);
	return cnt;
}

uint32_t uds_stream_write_le16(uds_stream_t *uds_stream, uint16_t val)
{
	uint32_t cnt = 0;

	cnt += uds_stream_write_byte(uds_stream, (uint8_t)val);
	cnt += uds_stream_write_byte(uds_stream, (uint8_t)(val >> 8));
	return cnt;
}

uint32_t uds_stream_write_le32(uds_stream_t *uds_stream, uint32_t val)
{
	uint32_t cnt = 0;

	cnt += uds_stream_write_le16(uds_stream, (uint16_t)(val));
	cnt += uds_stream_write_le16(uds_stream, (uint16_t)(val >> 16));
	return cnt;
}

uint32_t uds_stream_write_le64(uds_stream_t *uds_stream, uint64_t val)
{
	uint32_t cnt = 0;

	cnt += uds_stream_write_le32(uds_stream, (uint32_t)(val));
	cnt += uds_stream_write_le32(uds_stream, (uint32_t)(val >> 32));
	return cnt;
}

uint32_t uds_stream_write_data(uds_stream_t *uds_stream, uint8_t *data, uint32_t len)
{
	uint32_t cnt = 0;

	for (uint32_t i = 0; i < len; ++i) {
		if (uds_stream_left_len(uds_stream) > 0) {
			cnt += uds_stream_write_byte(uds_stream, data[i]);
		}
		else {
			break;
		}
	}
	return cnt;
}

uint32_t uds_stream_write_string(uds_stream_t *uds_stream, const char *str)
{
	return uds_stream_write_data(uds_stream, (uint8_t *)str, strlen(str));
}

uint8_t uds_stream_read_byte(uds_stream_t *uds_stream)
{
	uint8_t val = 0;

	if (uds_stream->cap - uds_stream->len >= 1) {
		val = *uds_stream->curr++;
		++uds_stream->len;
	}
	return val;
}

uint16_t uds_stream_read_be16(uds_stream_t *uds_stream)
{
	uint16_t val = 0;

	if (uds_stream->cap - uds_stream->len >= 2) {
		uint16_t hi = uds_stream_read_byte(uds_stream);
		uint16_t lo = uds_stream_read_byte(uds_stream);
		val = hi << 8 | lo;
	}
	return val;
}

uint32_t uds_stream_read_be32(uds_stream_t *uds_stream)
{
	uint32_t val = 0;

	if (uds_stream->cap - uds_stream->len >= 4) {
		uint32_t hi = uds_stream_read_be16(uds_stream);
		uint32_t lo = uds_stream_read_be16(uds_stream);
		val = hi << 16 | lo;
	}
	return val;
}

uint64_t uds_stream_read_be64(uds_stream_t *uds_stream)
{
	uint64_t val = 0;

	if (uds_stream->cap - uds_stream->len >= 1) {
		uint64_t hi = uds_stream_read_be32(uds_stream);
		uint64_t lo = uds_stream_read_be32(uds_stream);
		val = hi << 32 | lo;
	}
	return val;
}

uint16_t uds_stream_read_le16(uds_stream_t *uds_stream)
{
	uint16_t val = 0;

	if (uds_stream->cap - uds_stream->len >= 2) {
		uint16_t lo = uds_stream_read_byte(uds_stream);
		uint16_t hi = uds_stream_read_byte(uds_stream);
		val = hi << 8 | lo;
	}
	return val;
}

uint32_t uds_stream_read_le32(uds_stream_t *uds_stream)
{
	uint32_t val = 0;

	if (uds_stream->cap - uds_stream->len >= 4) {
		uint32_t lo = uds_stream_read_le16(uds_stream);
		uint32_t hi = uds_stream_read_le16(uds_stream);
		val = hi << 16 | lo;
	}
	return val;
}

uint64_t uds_stream_read_le64(uds_stream_t *uds_stream)
{
	uint64_t val = 0;

	if (uds_stream->cap - uds_stream->len >= 8) {
		uint64_t lo = uds_stream_read_le32(uds_stream);
		uint64_t hi = uds_stream_read_le32(uds_stream);
		val = hi << 32 | lo;
	}
	return val;
}

uint32_t uds_stream_read_data(uds_stream_t *uds_stream, uint8_t *buffer, uint32_t len)
{
	uint32_t i = 0;

	for (; i < len; ++i) {
		if (uds_stream_left_len(uds_stream) > 0) {
			buffer[i] = uds_stream_read_byte(uds_stream);
		}
		else {
			break;
		}
	}
	return i;
}
