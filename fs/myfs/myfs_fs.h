#define MYFS_BLOCKSIZE 4096
#define MYFS_DIRECT_BLOCK_NUM 3
#define MYFS_INDIRECT_BLOCK_NUM 2
#define MYFS_INDIRECT_DATA_BLOCK_NUM (MYFS_BLOCKSIZE / sizeof(__u32))

#define MYFS_FILE_NAME_LEN 28
#define MYFS_MAX_FILEBLOCKS (MYFS_DIRECT_BLOCK_NUM + (MYFS_INDIRECT_BLOCK_NUM * MYFS_INDIRECT_DATA_BLOCK_NUM))
#define MYFS_MAX_FILESIZE (MYFS_MAX_FILEBLOCKS * MYFS_BLOCKSIZE)

#define MYFS_SUPER_BLOCK 0
#define MYFS_I_BITMAP_BLOCK 1
#define MYFS_D_BITMAP_BLOCK 2
#define MYFS_INODE_BLOCK 3
#define MYFS_FIRST_DATA_BLOCK 4

#define MYFS_INODES_PER_BLOCK ((MYFS_BLOCKSIZE) / (sizeof(struct myfs_inode)))
#define MYFS_DIR_ENTRY_SIZE (sizeof(struct myfs_dir_entry))
#define MYFS_MAX_DIR_ENTRY_NUM ((MYFS_BLOCKSIZE) / MYFS_DIR_ENTRY_SIZE)

#define MYFS_MAGIC 0xababefef
#define MYFS_BLOCKSIZE_BITS 12

#define MYFS_ROOT_INO 1

/* myfs inode on disk */
struct myfs_inode {
	__u32 i_mode;
	__u32 i_size;
	__u32 i_direct_blocks[MYFS_DIRECT_BLOCK_NUM];
	__u32 i_indirect_blocks[MYFS_INDIRECT_BLOCK_NUM];
	__u32 i_dir_last_idx;
};

/* myfs super block on disk */
struct myfs_super_block {
	__u64 s_magic;
};

struct myfs_dir_entry {
	__u32 d_inode;
	char name[MYFS_FILE_NAME_LEN];
};
