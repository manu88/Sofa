/** 
 * @author Levente Kurusa <levex@linux.com> 
 * From https://github.com/levex/osdev/blob/master/include/ext2.h
 * **/
#pragma once

#include <stdint.h>
#include "IODevice.h"
#define EXT2_SIGNATURE 0xEF53

typedef struct {
	uint32_t inodes;
	uint32_t blocks;
	uint32_t reserved_for_root;
	uint32_t unallocatedblocks;
	uint32_t unallocatedinodes;
	uint32_t superblock_id;
	uint32_t blocksize_hint; // shift by 1024 to the left
	uint32_t fragmentsize_hint; // shift by 1024 to left
	uint32_t blocks_in_blockgroup;
	uint32_t frags_in_blockgroup;
	uint32_t inodes_in_blockgroup;
	uint32_t last_mount;
	uint32_t last_write;
	uint16_t mounts_since_last_check;
	uint16_t max_mounts_since_last_check;
	uint16_t ext2_sig; // 0xEF53
	uint16_t state;
	uint16_t op_on_err;
	uint16_t minor_version;
	uint32_t last_check;
	uint32_t max_time_in_checks;
	uint32_t os_id;
	uint32_t major_version;
	uint16_t uuid;
	uint16_t gid;
	uint8_t unused[940];
} __attribute__((packed)) superblock_t;

typedef struct {
	uint32_t block_of_block_usage_bitmap;
	uint32_t block_of_inode_usage_bitmap;
	uint32_t block_of_inode_table;
	uint16_t num_of_unalloc_block;
	uint16_t num_of_unalloc_inode;
	uint16_t num_of_dirs;
	uint8_t unused[14];
} __attribute__((packed)) block_group_desc_t;


#define INODE_TYPE_FIFO 0x1000
#define INODE_TYPE_CHAR_DEV 0x2000
#define INODE_TYPE_DIRECTORY 0x4000
#define INODE_TYPE_BLOCK_DEV 0x6000
#define INODE_TYPE_FILE 0x8000
#define INODE_TYPE_SYMLINK 0xA000
#define INODE_TYPE_SOCKET 0xC000
typedef struct {
	uint16_t type;
	uint16_t uid;
	uint32_t size;
	uint32_t last_access;
	uint32_t create_time;
	uint32_t last_modif;
	uint32_t delete_time;
	uint16_t gid;
	uint16_t hardlinks;
	uint32_t disk_sectors;
	uint32_t flags;
	uint32_t ossv1;
	uint32_t dbp[12];
	/*uint32_t dbp0;
	uint32_t dbp1;
	uint32_t dbp2;
	uint32_t dbp3;
	uint32_t dbp4;
	uint32_t dbp5;
	uint32_t dbp6;
	uint32_t dbp7;
	uint32_t dbp8;
	uint32_t dbp9;
	uint32_t dbp10;
	uint32_t dbp11;*/
	uint32_t singly_block;
	uint32_t doubly_block;
	uint32_t triply_block;
	uint32_t gen_no;
	uint32_t reserved1;
	uint32_t reserved2;
	uint32_t fragment_block;
	uint8_t ossv2[12];
} __attribute__((packed)) inode_t;

typedef struct __ext2_dir_entry {
	uint32_t inode;
	uint16_t size;
	uint8_t namelength;
	uint8_t reserved;
	/* name here */
} __attribute__((packed)) ext2_dir;

typedef struct __ext2_priv_data {
	superblock_t sb;
	uint32_t first_bgd;
	uint32_t number_of_bgs;
	uint32_t blocksize;
	uint32_t sectors_per_block;
	uint32_t inodes_per_block;
} __attribute__((packed)) ext2_priv_data;

uint8_t ext2_probe(IODevice* dev);
uint8_t ext2_mount(IODevice *dev, void *priv);
uint8_t ext2_find_file_inode(char *ff, inode_t *inode_buf, IODevice *dev, ext2_priv_data *priv);
uint32_t ext2_read_directory(char *filename, ext2_dir *dir, IODevice *dev, ext2_priv_data *priv);
uint8_t ext2_exist(char *file, IODevice *dev, ext2_priv_data *priv);
uint8_t ext2_read_root_directory(char *filename, IODevice *dev, ext2_priv_data *priv);
void ext2_list_directory(char *dd, char *buffer, IODevice *dev, ext2_priv_data *priv);

uint32_t ext2_get_inode_block(uint32_t inode, uint32_t *b, uint32_t *ioff, IODevice *dev, ext2_priv_data *priv);
uint8_t ext2_read_block(uint8_t *buf, uint32_t block, IODevice *dev, ext2_priv_data *priv);
