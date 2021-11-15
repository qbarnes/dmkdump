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


const char *
mode2str(uint8_t mode)
{
	const char *modestr[] = {"FM", "MFM", "RX02", "M2FM"};

	return modestr[mode];
}


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
dump_track(struct dmk_state *dmkst, int track, int side)
{
	uint8_t	*data = NULL;
	int	ret = 1;

	printf("Track %d, side %d", track, side);

	if (!dmk_seek(dmkst, track, side)) {
		printf(": [seek error!]\n");
		goto error;
	}

	int		sector;
	sector_info_t	si;

	for (sector = 0;
	     (sector < DMK_MAX_SECTOR) && dmk_read_id(dmkst, &si); ++sector);

	printf(" (%d sectors):", sector);

	if (!dmk_seek(dmkst, track, side)) {
		printf(": [seek error!]\n");
		goto error;
	}

	for (sector = 0;
	     (sector < DMK_MAX_SECTOR) && dmk_read_id(dmkst, &si); ++sector) {

		size_t	data_size = dmk_sector_size(&si);
		data = malloc(data_size);

		if (!data) {
			fprintf(stderr, "Failed to allocate %zu bytes for "
				"sector buffer.\n", data_size);
			goto error;
		}

		uint16_t actual_crc = 0, computed_crc = 0;

		if (dmk_read_sector_with_crcs(dmkst, &si, data,
						&actual_crc, &computed_crc)) {
			printf("\n  %s: cyl=%02x side=%02x sec=%02x "
			       "size=%02x [%zu] / ",
			        mode2str(si.mode),
				si.cylinder, si.head, si.sector,
				si.size_code, data_size);

			if (actual_crc == computed_crc) {
				printf("data crc=%04x\n", actual_crc);
			} else {
				printf("data "
				       "crcs=%04x:%04x(!)\n",
					actual_crc, computed_crc);
			}

			dump_sector_data(data, data_size);
		} else {
			printf("    Failed to read sector data.\n");
			fflush(stdout);
		}

		free(data);
		data = NULL;
	}

	printf("\n");
	ret = 0;

error:
	if (data)
		free(data);

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

		printf("%s: %d tracks [%s-sided/%s-density]\n\n",
			fn, tracks,
			ds ? "double" : "single",
			dd ? "double" : "single");
	}

	for (int t = 0; t < tracks; ++t) {
		for (int s = 0; s <= ds; ++s)
			dump_track(dmkst, t, s);
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
