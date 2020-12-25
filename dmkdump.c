/*
 * Dumps out the contents of a DMK floppy file.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "libdmk.h"


int
dump_sector_data(const uint8_t *data, size_t data_size)
{
	const unsigned int cols = 16;
	const unsigned int hcols = cols / 2;

	for (int i = 0; i < data_size; ++i) {
		unsigned int si = (i % cols) ?  (8 - !(i % hcols)) : !i;

		printf("%s%02x", "\n        " + si, data[i]);
	}
	printf("\n");

	return 0;
}


int
dump_track(struct dmk_state *dmkst, int side, int track)
{
	uint8_t	*data = NULL;
	int	ret = 1;

	printf("Track %d, side %d:\n", track, side);

	if (!dmk_seek(dmkst, track, side)) {
		printf(": [seek error]\n");
		goto error;
	}

	int		sector;
	sector_info_t	si;

	for (sector = 0;
	     (sector < DMK_MAX_SECTOR) && dmk_read_id(dmkst, &si); ++sector) {

		printf("    cyl=%02x side=%02x sec=%02x size=%02x\n",
			si.cylinder, si.head, si.sector, si.size_code);

		size_t	data_size = 128 << si.size_code;
		data = malloc(data_size);

		if (!data) {
			fprintf(stderr, "Failed to allocate %lu bytes for "
				"sector buffer.\n", data_size);
			goto error;
		}

	if (dmk_read_sector(dmkst, &si, data)) {
			dump_sector_data(data, data_size);
		} else {
			printf("Failed to read sector data.\n");
			goto error;
		}

		free(data);
		data = NULL;
	}

	printf("    Sectors found: %d\n", sector);
	ret = 0;

error:
	if (data)
		free (data);
	return ret;
}


int
main(int argc, char **argv)
{
	char	*fn;
	struct dmk_state *dmkst;
	int	tracks, ds, dd;

	if (argc > 1) {
		fn = argv[1];

		dmkst = dmk_open_image(fn, 0, &ds, &tracks, &dd);
		if (!dmkst) {
			fprintf(stderr, "Failed to open '%s' (%d [%s]).\n",
				fn, errno, strerror(errno));
			return 2;
		}

		printf("%s: %d tracks [%s-sided/%s-density]\n",
			fn, tracks,
			ds ? "double" : "single",
			dd ? "double" : "single");

	} else {
		fprintf(stderr, "Must provide DMK file name argument.\n");
		return 1;
	}

	for (int s = 0; s <= ds; ++s) {
		for (int t = 0; t < tracks; ++t)
			dump_track(dmkst, s, t);
	}

	if (!dmk_close_image(dmkst)) {
		fprintf(stderr, "Close of '%s' failed.\n", fn);
		return 1;
	}

	return 0;
}
