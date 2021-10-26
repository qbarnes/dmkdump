/*
 * Dumps out the contents of a DMK floppy file.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include "libdmk.h"


void
dump_sector_data(const uint8_t *data, size_t data_size)
{
	unsigned int	cols = 16;
	unsigned int	hcols = cols / 2;
	char		asc_buf[cols+1];

	asc_buf[cols] = '\0';

	// Print out indented byte columns in hex.  Put an extra
	// space in the middle of the columns.
	for (int i = 0; i < data_size; ++i) {
		unsigned int si = (i % cols) ? (4 - !(i % hcols)) : !i;
		printf("\n    %02x" + si, data[i]);

		int abi = (i % cols);
		asc_buf[abi] = isprint(data[i]) ? data[i] : '.';
		if (abi == cols - 1)
			printf("    %s", asc_buf);
	}

	printf("\n");
}


int
dump_track(struct dmk_state *dmkst, int side, int track)
{
	uint8_t	*data = NULL;
	int	ret = 1;

	printf("Track %d, side %d:", track, side);

	if (!dmk_seek(dmkst, track, side)) {
		printf(": [seek error]\n");
		goto error;
	}

	int		sector;
	sector_info_t	si;

	for (sector = 0;
	     (sector < DMK_MAX_SECTOR) && dmk_read_id(dmkst, &si); ++sector) {

		size_t	data_size = dmk_sector_size(&si);
		printf("\n  cyl=%02x side=%02x sec=%02x size=%02x [%zu]\n",
			si.cylinder, si.head, si.sector,
			si.size_code, data_size);

		//size_t	data_size = 256 << (si.size_code & 0x3);
		//size_t	data_size = 1024;
		//size_t	data_size = 128 << si.size_code;
		data = malloc(data_size);

		if (!data) {
			fprintf(stderr, "Failed to allocate %lu bytes for "
				"sector buffer.\n", data_size);
			goto error;
		}

		if (dmk_read_sector(dmkst, &si, data)) {
			dump_sector_data(data, data_size);
		} else {
			printf("    Failed to read sector data.\n");
			fflush(stdout);
		}

		free(data);
		data = NULL;
	}

	printf("    Sectors found: %d\n\n", sector);
	ret = 0;

error:
	if (data)
		free (data);
	return ret;
}


int
process_dmks(char **dmk_list)
{
	char	*fn;
	struct dmk_state *dmkst;
	int	tracks, ds, dd;

	while((fn = *dmk_list++)) {
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

		printf("0-%s%s-%dT\n",
			ds ? "DS" : "SS", dd ? "DD" : "SD", tracks);
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


int
main(int argc, char **argv)
{
	int	ret = 1;

	if (argc > 1) {
		ret = process_dmks(&argv[1]);
	} else {
		fprintf(stderr, "Must provide DMK file name argument.\n");
	}

	return ret;
}
