/*
 * volume_id - reads filesystem label and uuid
 *
 * Copyright (C) 2005 Kay Sievers <kay.sievers@vrfy.org>
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation; either
 *	version 2.1 of the License, or (at your option) any later version.
 *
 *	This library is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *	Lesser General Public License for more details.
 *
 *	You should have received a copy of the GNU Lesser General Public
 *	License along with this library; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef __VOLUME_ID_INTERNAL__
#define __VOLUME_ID_INTERNAL__

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sys/mount.h>
//#include <sys/endian.h>
#include  <endian.h>
#include <byteswap.h>

//#include <typesizes.h>
#include "volume_id.h"
#include "xfuncs_printf.h"

#if 0
#define bswap_64 __bswap64
#define bswap_32 __bswap32
#define bswap_16 __bswap16
#endif


#define ARRAY_SIZE(x) ((unsigned)(sizeof(x) / sizeof((x)[0])))
//PUSH_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN

#define dbg(...) ((void)0)
/* #define dbg(...) bb_error_msg(__VA_ARGS__) */

/* volume_id.h */

#define VOLUME_ID_VERSION		48

#define VOLUME_ID_LABEL_SIZE		64
#define VOLUME_ID_UUID_SIZE		36
#define VOLUME_ID_FORMAT_SIZE		32
#define VOLUME_ID_PARTITIONS_MAX	256

#define PACKED __attribute__ ((packed))

enum volume_id_usage {
	VOLUME_ID_UNUSED,
	VOLUME_ID_UNPROBED,
	VOLUME_ID_OTHER,
	VOLUME_ID_FILESYSTEM,
	VOLUME_ID_PARTITIONTABLE,
	VOLUME_ID_RAID,
	VOLUME_ID_DISKLABEL,
	VOLUME_ID_CRYPTO,
};

#ifdef UNUSED_PARTITION_CODE
struct volume_id_partition {
//	const char	*type;
//	const char	*usage;
//	smallint	usage_id;
//	uint8_t		pt_type_raw;
//	uint64_t	pt_off;
//	uint64_t	pt_len;
};
#endif

struct volume_id {
	int		fd;
//	int		fd_close:1;
	int		error;
	size_t		sbbuf_len;
	size_t		seekbuf_len;
	uint8_t		*sbbuf;
	uint8_t		*seekbuf;
	uint64_t	seekbuf_off;
#ifdef UNUSED_PARTITION_CODE
	struct volume_id_partition *partitions;
	size_t		partition_count;
#endif
//	uint8_t		label_raw[VOLUME_ID_LABEL_SIZE];
//	size_t		label_raw_len;
	char		label[VOLUME_ID_LABEL_SIZE+1];
//	uint8_t		uuid_raw[VOLUME_ID_UUID_SIZE];
//	size_t		uuid_raw_len;
	/* uuid is stored in ASCII (not binary) form here: */
	char		uuid[VOLUME_ID_UUID_SIZE+1];
//	char		type_version[VOLUME_ID_FORMAT_SIZE];
//	smallint	usage_id;
//	const char	*usage;
#if 1 //ENABLE_FEATURE_BLKID_TYPE
	char	type[10];
#endif
};

struct volume_id * volume_id_open_node(int fd);
int volume_id_probe_all(struct volume_id *id, /*uint64_t off,*/ uint64_t size);
void free_volume_id(struct volume_id *id);

/* util.h */

/* size of superblock buffer, reiserfs block is at 64k */
#define SB_BUFFER_SIZE				0x11000
/* size of seek buffer, FAT cluster is 32k max */
#define SEEK_BUFFER_SIZE			0x10000

#if BB_LITTLE_ENDIAN
# define le16_to_cpu(x) (uint16_t)(x)
# define le32_to_cpu(x) (uint32_t)(x)
# define le64_to_cpu(x) (uint64_t)(x)
# define be16_to_cpu(x) (uint16_t)(bswap_16(x))
# define be32_to_cpu(x) (uint32_t)(bswap_32(x))
# define cpu_to_le16(x) (uint16_t)(x)
# define cpu_to_le32(x) (uint32_t)(x)
# define cpu_to_be32(x) (uint32_t)(bswap_32(x))
#else
# define le16_to_cpu(x) (uint16_t)(bswap_16(x))
# define le32_to_cpu(x) (uint32_t)(bswap_32(x))
# define le64_to_cpu(x) (uint64_t)(bb_bswap_64(x))
# define be16_to_cpu(x) (uint16_t)(x)
# define be32_to_cpu(x) (uint32_t)(x)
# define cpu_to_le16(x) (uint16_t)(bswap_16(x))
# define cpu_to_le32(x) (uint32_t)(bswap_32(x))
# define cpu_to_be32(x) (uint32_t)(x)
#endif

/* volume_id_set_uuid(id,buf,fmt) assumes size of uuid buf
 * by shifting: 4 << fmt, except for fmt == UUID_DCE_STRING.
 * The constants below should match sizes.
 */
enum uuid_format {
	UUID_DOS = 0,		/* 4 bytes */
	UUID_NTFS = 1,		/* 8 bytes */
	UUID_DCE = 2,		/* 16 bytes */
	UUID_DCE_STRING = 3,	/* 36 bytes (VOLUME_ID_UUID_SIZE) */
};

enum endian {
	LE = 0,
	BE = 1
};


void volume_id_free_buffer(struct volume_id *id);
int  volume_id_probe_exfat(struct volume_id *id);
void *volume_id_get_buffer(struct volume_id *id, uint64_t off, size_t len);
void volume_id_set_unicode16(char *str, size_t len, const uint8_t *buf, enum endian endianess, size_t count);
void volume_id_set_label_unicode16(struct volume_id *id, const uint8_t *buf, enum endian endianess, size_t count);
void volume_id_set_uuid(struct volume_id *id, const uint8_t *buf, enum uuid_format format);

#endif
