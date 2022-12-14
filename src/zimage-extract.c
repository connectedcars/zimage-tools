#define _GNU_SOURCE

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

// https://gist.github.com/atr000/249599
#define bswap_16(value) \
	((((value)&0xff) << 8) | ((value) >> 8))

#define bswap_32(value)                                       \
	(((uint32_t)bswap_16((uint16_t)((value)&0xffff)) << 16) | \
	 (uint32_t)bswap_16((uint16_t)((value) >> 16)))

// From ELL - https://git.kernel.org/pub/scm/libs/ell/ell.git/
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define LE16_TO_CPU(val) (val)
#define LE32_TO_CPU(val) (val)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define LE16_TO_CPU(val) bswap_16(val)
#define LE32_TO_CPU(val) bswap_32(val)
#else
#error "Unknown byte order"
#endif

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

enum ZIMAGE_FORMAT
{
	ZIMAGE_UNKNOWN,
	ZIMAGE_GZIP, /**< gzip compressed zimage payload */
	ZIMAGE_LZO,	 /**< lzo compressed zimage payload */
	ZIMAGE_XZ,	 /**< xz compressed zimage payload */
	ZIMAGE_LZMA, /**< lzma compressed zimage payload */
	ZIMAGE_LZ4,	 /**< lz4 compressed zimage payload */
	ZIMAGE_ZSTD, /**< zstd compressed zimage payload */
};

struct zimage_header
{
	uint32_t magic; /**< magic - always 0x4b 0x4c 0x53 0x5a */
	uint32_t end;	/**< offset to end of payload */
	uint32_t bss;	/**< size of zero-initialized rw data */
	uint32_t zero;	/**< always zero */
};

static const uint8_t magic_gzip[] = {0x1f, 0x8b, 0x08};
static const uint8_t magic_lzo[] = {0x89, 0x4c, 0x5a, 0x4f, 0x00, 0x0d, 0x0a, 0x1a, 0x0a};
static const uint8_t magic_xz[] = {0xfd, 0x37, 0x7a, 0x58, 0x5a, 0x00};
static const uint8_t magic_lzma[] = {0x5d, 0x00, 0x00};
static const uint8_t magic_lz4[] = {0x02, 0x21, 0x4c, 0x18};
static const uint8_t magic_zstd[] = {0xFD, 0x2F, 0xB5, 0x28};

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "zimage-extract zImage\n");
		exit(1);
	}

	char *path = argv[1];

	int fd = -1;
	size_t size = 0;
	size_t offset = 0;
	struct stat st = {0};

	uint8_t *zimage_data_map;

	struct zimage_header *header;
	size_t header_offset;

	enum ZIMAGE_FORMAT format = ZIMAGE_UNKNOWN;
	uint8_t *payload_match;
	uint8_t *payload;
	size_t payload_offset;
	uint32_t payload_end;
	size_t payload_max_size;
	size_t payload_size;

	FILE *f;
	char buf[2048];

	memset(buf, 0, sizeof(buf));

	if (stat(path, &st) != 0)
	{
		fprintf(stderr, "stat: failed to open file: %s: %s", path, strerror(errno));
		exit(255);
	}
	size = st.st_size;

	fd = open(path, O_RDONLY);
	if (fd < 0)
	{
		fprintf(stderr, "file: failed to open file: %s: %s", path, strerror(errno));
		exit(255);
	}

	zimage_data_map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, offset);
	if (zimage_data_map == MAP_FAILED)
	{
		fprintf(stderr, "file: failed to create memory map: %s: %s", path, strerror(errno));
		exit(255);
	}

	char magic[] = {0x4b, 0x4c, 0x53, 0x5a};

	header = memmem(zimage_data_map, size, magic, sizeof(magic));
	if (header == NULL)
	{
		fprintf(stderr, "zimage: no header found\n");
		exit(255);
	}
	header_offset = ((uint8_t *)header - zimage_data_map);
	payload_end = LE32_TO_CPU(header->end);
	payload_max_size = payload_end - header_offset;
	payload = (uint8_t *)header + payload_end; // Set payload to end of payload

	// https://github.com/connectedcars/firmware_ccupd/blob/2740c3ab3ddd149fa839843dfecbeb818bec4f8b/src/method/zimage.c#L115

	// fprintf(stderr, "Found zimage header offset %zu\n", header_offset);

	payload_match = memmem(header, payload_max_size, magic_gzip, sizeof(magic_gzip));
	if (payload_match != NULL)
	{
		payload = payload_match;
		format = ZIMAGE_GZIP;
	}
	payload_match = memmem(header, payload_max_size, magic_lzo, sizeof(magic_lzo));
	if (payload_match != NULL)
	{
		// special case on lzo, as there is another lzo-magic placed just after the zimage header
		payload_match = memmem(payload_match + 4, payload_max_size - 4, magic_lzo, sizeof(magic_lzo));
		if (payload_match != NULL && payload_match < payload) // Only pick better magic if it's located before other magic found
		{
			payload = payload_match;
			format = ZIMAGE_LZO;
		}
	}
	payload_match = memmem(header, payload_max_size, magic_xz, sizeof(magic_xz));
	if (payload_match != NULL && payload_match < payload) // Only pick better magic if it's located before other magic found
	{
		payload = payload_match;
		format = ZIMAGE_XZ;
	}
	payload_match = memmem(header, payload_max_size, magic_lzma, sizeof(magic_lzma));
	if (payload_match != NULL && payload_match < payload) // Only pick better magic if it's located before other magic found
	{
		payload = payload_match;
		format = ZIMAGE_LZMA;
	}
	payload_match = memmem(header, payload_max_size, magic_lz4, sizeof(magic_lz4));
	if (payload_match != NULL && payload_match < payload) // Only pick better magic if it's located before other magic found
	{
		payload = payload_match;
		format = ZIMAGE_LZ4;
	}
	payload_match = memmem(header, payload_max_size, magic_zstd, sizeof(magic_zstd));
	if (payload_match != NULL && payload_match < payload) // Only pick better magic if it's located before other magic found
	{
		payload = payload_match;
		format = ZIMAGE_ZSTD;
	}

	if (format == ZIMAGE_UNKNOWN)
	{
		fprintf(stderr, "zimage: no known format found\n");
		exit(255);
	}

	payload_offset = ((uint8_t *)payload - zimage_data_map);
	payload_size = payload_end - payload_offset;
	// fprintf(stderr, "Found payload at offset %zu with format %u with size %zu\n", payload_offset, format, payload_size);

	char *decompression_command;
	switch (format)
	{
	case ZIMAGE_LZO:
	{
		decompression_command = "lzop -d";
		break;
	}
	case ZIMAGE_GZIP:
	{
		decompression_command = "gzip -d";
		exit(255);
	}
	case ZIMAGE_LZ4:
	{
		decompression_command = "lz4 -d";
		exit(255);
	}
	case ZIMAGE_LZMA:
	{
		decompression_command = "lzma -d";
		exit(255);
	}
	case ZIMAGE_XZ:
	{
		decompression_command = "xz -d";
		exit(255);
	}
	case ZIMAGE_ZSTD:
	{
		decompression_command = "zstd -d";
		exit(255);
	}
	default:
		fprintf(stderr, "zimage: unknown format\n");
		exit(255);
	}

	// Open decompression command and write payload to stdin
	f = popen(decompression_command, "w");
	if (f == NULL)
	{
		fprintf(stderr, "failed to start decompression command: %s\n", decompression_command);
		exit(255);
	}
	int d = fileno(f);
	int write_size = payload_size;
	ssize_t res;
	while (write_size > 0)
	{
		res = write(d, payload, MIN(write_size, 4096));
		write_size -= res;
		payload += res;
	}
	close(fd);

	munmap(zimage_data_map, size);
}
