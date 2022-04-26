#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pagemap.h>
#include <linux/buffer_head.h>
#include <linux/slab.h>
#include <linux/vfs.h>
#include <linux/types.h>
#include "myfs_fs.h"

#define DEBUG 0

/* myfs inode data in memory */
struct myfs_inode_info {
	struct inode vfs_inode;
	__u32 dir_last_idx;
	__u32 direct_blocks[MYFS_DIRECT_BLOCK_NUM];
	__u32 indirect_blocks[MYFS_INDIRECT_BLOCK_NUM];
};

struct myfs_indirect_block {
        __u32 block[MYFS_INDIRECT_DATA_BLOCK_NUM];
};

/* myfs superblock data in memory */
struct myfs_sb_info {
	struct super_block *sb;
	unsigned char i_bitmap[MYFS_BLOCKSIZE];
	unsigned char d_bitmap[MYFS_BLOCKSIZE];
};


// inode.c
extern void myfs_set_inode(struct inode *inode);
extern struct myfs_inode *myfs_raw_inode(struct super_block *sb, ino_t ino, struct buffer_head **bh);
extern struct inode *myfs_iget(struct super_block *, unsigned long ino);

// bitmap.c
extern struct inode *myfs_new_inode(const struct inode *dir, umode_t mode);
extern int get_block_location(struct inode *inode, int num);
extern int put_block(struct super_block *sb, int num);
extern int get_free_block(struct super_block *sb);
extern int put_inode(struct super_block *sb, int num);
extern int get_free_inode(struct super_block *sb);
extern int allocate_new_block(struct inode *inode, int block, int *new_block);
extern int deallocate_block(struct inode *inode, int block);
extern int myfs_get_block(struct inode *inode, blkcnt_t block, struct buffer_head *bh_result, int create);

// file.c
extern const struct inode_operations myfs_file_inode_operations;
extern const struct file_operations myfs_file_operations;
extern const struct address_space_operations myfs_aops;

//dir.c
extern const struct file_operations myfs_dir_operations;
extern const struct inode_operations myfs_dir_inode_operations;

static inline struct myfs_sb_info *myfs_sb(struct super_block *sb)
{
        return sb->s_fs_info;
}

static inline struct myfs_inode_info *myfs_i(struct inode *inode)
{
        return container_of(inode, struct myfs_inode_info, vfs_inode);
}
