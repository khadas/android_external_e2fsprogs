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

//kbuild:lib-$(CONFIG_VOLUMEID) += volume_id.o util.o

#include "volume_id_internal.h"


/* Some detection routines do not set label or uuid anyway,
 * so they are disabled. */

/* Looks for partitions, we don't use it: */
#define ENABLE_FEATURE_VOLUMEID_MAC           0
/* #define ENABLE_FEATURE_VOLUMEID_MSDOS      0 - NB: this one
 * was not properly added to probe table anyway - ??! */

/* None of RAIDs have label or uuid, except LinuxRAID: */
#define ENABLE_FEATURE_VOLUMEID_HIGHPOINTRAID 0
#define ENABLE_FEATURE_VOLUMEID_ISWRAID       0
#define ENABLE_FEATURE_VOLUMEID_LSIRAID       0
#define ENABLE_FEATURE_VOLUMEID_LVM           0
#define ENABLE_FEATURE_VOLUMEID_NVIDIARAID    0
#define ENABLE_FEATURE_VOLUMEID_PROMISERAID   0
#define ENABLE_FEATURE_VOLUMEID_SILICONRAID   0
#define ENABLE_FEATURE_VOLUMEID_VIARAID       0

/* These filesystems also have no label or uuid: */
#define ENABLE_FEATURE_VOLUMEID_MINIX         0
#define ENABLE_FEATURE_VOLUMEID_HPFS          0
#define ENABLE_FEATURE_VOLUMEID_UFS           0


typedef int (*probe_fptr)(struct volume_id *id /*, uint64_t off*/);



/* signature in the first block, only small buffer needed */
static const probe_fptr fs1[] = {
	volume_id_probe_exfat,
};


int volume_id_probe_all(struct volume_id *id, /*uint64_t off,*/ uint64_t size)
{
	unsigned i, arsize;

	/* signature in the first block, only small buffer needed */
	arsize=ARRAY_SIZE(fs1);
	for (i = 0; i < arsize; i++) {
                int r = -1;
		if (fs1[i](id /*,off*/) == 0) {
			goto ret;
                }
		if (id->error) {
			goto ret;
                }
	}

	/* fill buffer with maximum */
	volume_id_get_buffer(id, 0, SB_BUFFER_SIZE);
	volume_id_free_buffer(id);
ret:
	return (- id->error); /* 0 or -1 */
}


/* open volume by device node */
struct volume_id*  volume_id_open_node(int fd)
{
	struct volume_id *id;

	id = xzalloc(sizeof(struct volume_id));
	id->fd = fd;
	///* close fd on device close */
	//id->fd_close = 1;
	return id;
}

#ifdef UNUSED
/* open volume by major/minor */
struct volume_id*  volume_id_open_dev_t(dev_t devt)
{
	struct volume_id *id;
	char *tmp_node[VOLUME_ID_PATH_MAX];

	tmp_node = xasprintf("/dev/.volume_id-%u-%u-%u",
		(unsigned)getpid(), (unsigned)major(devt), (unsigned)minor(devt));

	/* create temporary node to open block device */
	unlink(tmp_node);
	if (mknod(tmp_node, (S_IFBLK | 0600), devt) != 0)
		bb_perror_msg_and_die("can't mknod(%s)", tmp_node);

	id = volume_id_open_node(tmp_node);
	unlink(tmp_node);
	free(tmp_node);
	return id;
}
#endif

void  free_volume_id(struct volume_id *id)
{
	if (id == NULL)
		return;

	//if (id->fd_close != 0) - always true
		close(id->fd);
	volume_id_free_buffer(id);
#ifdef UNUSED_PARTITION_CODE
	free(id->partitions);
#endif
	free(id);
}
