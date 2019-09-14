/// AUTOGENERATED TEST ///
#define LFS_YES_TRACE 1

#include "lftl.h"
#include "emubd/lfs_emubd.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



// lfs declarations
lfs_t lfs;
lfs_emubd_t bd;
// other declarations for convenience
lfs_file_t file;
struct lfs_info info;
uint8_t buffer[1024];
char path[1024];

// test configuration options
#ifndef LFS_READ_SIZE
#define LFS_READ_SIZE 64
#endif

#ifndef LFS_PROG_SIZE
#define LFS_PROG_SIZE LFS_READ_SIZE
#endif

#ifndef LFS_BLOCK_SIZE
#define LFS_BLOCK_SIZE 512
#endif

#ifndef LFS_BLOCK_COUNT
#define LFS_BLOCK_COUNT 1024
#endif

#ifndef LFS_BLOCK_CYCLES
#define LFS_BLOCK_CYCLES 1024
#endif

#ifndef LFS_CACHE_SIZE
#define LFS_CACHE_SIZE (64 % LFS_PROG_SIZE == 0 ? 64 : LFS_PROG_SIZE)
#endif

#ifndef LFS_LOOKAHEAD_SIZE
#define LFS_LOOKAHEAD_SIZE 16
#endif
/*
const struct lfs_config cfg = { {
	.context = &bd,
	.read = &lfs_emubd_read,
	.prog = &lfs_emubd_prog,
	.erase = &lfs_emubd_erase,
	.sync = &lfs_emubd_sync,

	.read_size = LFS_READ_SIZE,
	.prog_size = LFS_PROG_SIZE,
	.block_size = LFS_BLOCK_SIZE,
	.block_count = LFS_BLOCK_COUNT,
	.block_cycles = LFS_BLOCK_CYCLES,
	.cache_size = LFS_CACHE_SIZE,
	.lookahead_size = LFS_LOOKAHEAD_SIZE,
} };
*/
struct lfs_config cfg;
static void cfg_init()
{
	cfg.context = &bd;
	cfg.read = &lfs_emubd_read;
	cfg.prog = &lfs_emubd_prog;
	cfg.erase = &lfs_emubd_erase;
	cfg.sync = &lfs_emubd_sync;

	cfg.read_size = LFS_READ_SIZE;
	cfg.prog_size = LFS_PROG_SIZE;
	cfg.block_size = LFS_BLOCK_SIZE;
	cfg.block_count = LFS_BLOCK_COUNT;
	cfg.block_cycles = LFS_BLOCK_CYCLES;
	cfg.cache_size = LFS_CACHE_SIZE;
	cfg.lookahead_size = LFS_LOOKAHEAD_SIZE;
}


static void seq_gen(uint32_t seed, uint8_t *buf, size_t length)
{
	size_t i;

	srand(seed);
	for (i = 0; i < length; i++)
		buf[i] = (uint8_t)(rand() & 0xFF);
}
static void seq_assert(unsigned int seed, const uint8_t *buf, size_t length)
{
	size_t i;

	srand(seed);
	for (i = 0; i < length; i++) {
		const uint8_t expect = (uint8_t)(rand() & 0xFF);

		if (buf[i] != expect) {
			fprintf(stderr, "seq_assert: mismatch at %ld in "
				"sequence %d: 0x%02x (expected 0x%02x)\n",
				i, seed, buf[i], expect);
			abort();
		}
	}
}


#define NUM_SECTORS		16
static uint32_t sector_list[NUM_SECTORS];
static void shuffle(int seed)
{
	int i;

	srand(seed);
	for (i = 0; i < NUM_SECTORS; i++)
		sector_list[i] = i;

	for (i = NUM_SECTORS - 1; i > 0; i--) {
		const int j = rand() % i;
		const int tmp = sector_list[i];

		sector_list[i] = sector_list[j];
		sector_list[j] = tmp;
	}
}
// Entry point
int main(void) {
	{
		uint8_t wbuffer[1024], rbuffer[1024];
		int err;

		cfg_init();

		lfs_emubd_create(&cfg, "blocks");

		printf("Map init\n");
		lftl_map_mount(&cfg);
		printf("\n");

		//*
		printf("Writing sectors...\n");
		shuffle(0);
		for (int i = 0; i < NUM_SECTORS; i++) {
			const uint32_t s = sector_list[i];
			seq_gen(s, wbuffer, cfg.block_size);
			err = lftl_map_write(s, wbuffer);
		}
		printf("\n");

		printf("Read back...\n");
		shuffle(1);
		for (int i = 0; i < NUM_SECTORS; i++) {
			const uint32_t s = sector_list[i];
			err = lftl_map_read(s, rbuffer);
			seq_assert(s, rbuffer, cfg.block_size);
		}
		printf("\n");
		//*/
		printf("Print the map ...\n");
		for (int i = 0; i < NUM_SECTORS; i++) {
			uint32_t local_block;
			err = lftl_map_find(i, &local_block);
			if (err == 0)
			{
				printf("Logical sector %04d ==> %04d  physical block\n", i, local_block);
			}
		}
		printf("\n");

	
	
		lftl_map_unmount();
		lfs_emubd_destroy(&cfg);
	}
}