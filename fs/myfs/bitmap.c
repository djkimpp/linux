#include "myfs.h"

int get_block_location(struct inode *inode, int num)
{
        struct myfs_inode_info *inode_info = myfs_i(inode);
        struct buffer_head *bh;
        struct myfs_indirect_block *ib;
        int ret;
        int indirect_num, indirect_idx;
#if DEBUG
	printk("MYFS : %s block num : %d\n",__func__, num);
#endif

        if(num < MYFS_DIRECT_BLOCK_NUM) {
#if DEBUG
	printk("MYFS : %s return value %d\n",__func__, inode_info->direct_blocks[num]);
#endif
                return inode_info->direct_blocks[num];
	}
        indirect_num = (num - MYFS_DIRECT_BLOCK_NUM) / MYFS_INDIRECT_DATA_BLOCK_NUM;
        indirect_idx = (num - MYFS_DIRECT_BLOCK_NUM) % MYFS_INDIRECT_DATA_BLOCK_NUM;
#if DEBUG
	printk("MYFS : %s indirect block[%d][%d]\n",__func__, indirect_num, indirect_idx);
#endif

        bh = sb_bread(inode->i_sb, inode_info->indirect_blocks[indirect_num]);
        ib = (struct myfs_indirect_block *)bh->b_data;
        ret = ib->block[indirect_idx];
        brelse(bh);
#if DEBUG
	printk("MYFS : %s return value %d\n",__func__, ret);
#endif

        return ret;
}

int put_block(struct super_block *sb, int num)
{
        struct myfs_sb_info *sbi = myfs_sb(sb);
        int ret = -EFAULT;
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        if(test_and_clear_bit(num, (unsigned long *)sbi->d_bitmap))
                ret = 0;
#if DEBUG
	printk("MYFS : %s return value %d\n",__func__, ret);
#endif

        return ret;
}

int get_free_block(struct super_block *sb)
{
        struct myfs_sb_info *sbi = myfs_sb(sb);
        int max_bit = 8 * MYFS_BLOCKSIZE;
        unsigned long idx;
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        idx = find_first_zero_bit((unsigned long*)sbi->d_bitmap, max_bit);
        if(idx >= max_bit)
                return -1;
        set_bit(idx, (unsigned long*)sbi->d_bitmap);
        return idx;
}

int put_inode(struct super_block *sb, int num)
{
        struct myfs_sb_info *sbi = myfs_sb(sb);
        int ret = -EFAULT;
#if DEBUG
	printk("MYFS : %s inode num : %d\n",__func__, num);
#endif

        if(test_and_clear_bit(num, (unsigned long *)sbi->i_bitmap))
                ret = 0;
#if DEBUG
	printk("MYFS : %s return value %d\n",__func__, ret);
#endif

        return ret;
}

int get_free_inode(struct super_block *sb)
{
        struct myfs_sb_info *sbi = myfs_sb(sb);
        int max_bit = MYFS_INODES_PER_BLOCK;
        unsigned long idx;
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        idx = find_first_zero_bit((unsigned long*)sbi->i_bitmap, max_bit);
        if(idx >= max_bit)
                return -1;
        set_bit(idx, (unsigned long*)sbi->i_bitmap);
#if DEBUG
	printk("MYFS : %s return value is %ld\n",__func__, idx);
#endif
        return idx;
}

struct inode *myfs_new_inode(const struct inode *dir, umode_t mode)
{
        struct super_block *sb = dir->i_sb;
        struct inode *inode = new_inode(sb);
        unsigned long ino;
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif
        if (!inode)
                return NULL;

        ino = get_free_inode(sb);
        if(ino < 0)
                return NULL;

        inode_init_owner(&init_user_ns, inode, dir, mode);
        inode->i_ino = ino;
        inode->i_blocks = 0;

        insert_inode_hash(inode);

        return inode;
}

int allocate_new_block(struct inode *inode, int block, int *new_block)
{
        struct super_block *sb = inode->i_sb;
        struct myfs_inode_info *inode_info = myfs_i(inode);
        int indirect_num, indirect_idx;
        int new = 0;
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        new = get_block_location(inode, block);
        if(new != 0)
                return 0;

        if(block < MYFS_DIRECT_BLOCK_NUM) {
                new = get_free_block(sb);
                if(new == -1)
                        return -ENOMEM;
                inode_info->direct_blocks[block] = new;
        } else {
                indirect_num = (block - MYFS_DIRECT_BLOCK_NUM) / MYFS_INDIRECT_DATA_BLOCK_NUM;
                indirect_idx = (block - MYFS_DIRECT_BLOCK_NUM) % MYFS_INDIRECT_DATA_BLOCK_NUM;

                if(inode_info->indirect_blocks[indirect_num] == 0)
                {
                        new = get_free_block(sb);
                        if(new == -1)
                                return -ENOMEM;
                        inode_info->indirect_blocks[indirect_num] = new;
                }

                new = get_free_block(sb);
                if(new == -1)
                        return -ENOMEM;
        }

        *new_block = new;
        return 0;
}

int deallocate_block(struct inode *inode, int block)
{
        struct buffer_head *bh;
        struct myfs_indirect_block *ib;
        struct super_block *sb = inode->i_sb;
        struct myfs_inode_info *inode_info = myfs_i(inode);
        int indirect_num, indirect_idx;
        int loc;
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        loc = get_block_location(inode, block);
        if(loc == 0)
                return 0;

        if(block < MYFS_DIRECT_BLOCK_NUM) {
                inode_info->direct_blocks[block] = 0;
        } else {
                indirect_num = (block - MYFS_DIRECT_BLOCK_NUM) / MYFS_INDIRECT_DATA_BLOCK_NUM;
                indirect_idx = (block - MYFS_DIRECT_BLOCK_NUM) % MYFS_INDIRECT_DATA_BLOCK_NUM;
                bh = sb_bread(sb, inode_info->indirect_blocks[indirect_num]);
                ib = (struct myfs_indirect_block *)bh->b_data;
                ib->block[indirect_idx] = 0;
                brelse(bh);

                if(indirect_idx == 0) {
                        __u32 old_indirect_block = inode_info->indirect_blocks[indirect_num];
                        inode_info->indirect_blocks[indirect_num] = 0;
                        put_block(sb, old_indirect_block);
                }
        }
        put_block(sb, loc);

        return 0;
}

int myfs_get_block(struct inode *inode, blkcnt_t block, struct buffer_head *bh_result, int create)
{
        struct super_block *sb = inode->i_sb;
        int err;
        int new_block;
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        if(block > MYFS_MAX_FILEBLOCKS - 1)
                return -EFBIG;

        new_block = get_block_location(inode, block);
        if(new_block != 0)
        {
                map_bh(bh_result, sb, new_block);
                return 0;
        }

        if(!create)
                return 0;

        err = allocate_new_block(inode, block, &new_block);
        if(err)
                return err;
        set_buffer_new(bh_result);
        map_bh(bh_result, sb, new_block);
        
        return 0;
}
