#include "myfs.h"

void myfs_set_inode(struct inode *inode)
{
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        if (S_ISREG(inode->i_mode)) {
                inode->i_op = &myfs_file_inode_operations;
                inode->i_fop = &myfs_file_operations;
        } else if (S_ISDIR(inode->i_mode)) {
                inode->i_op = &myfs_dir_inode_operations;
                inode->i_fop = &myfs_dir_operations;
		inc_nlink(inode);
        }
        inode->i_mapping->a_ops = &myfs_aops;
}

struct myfs_inode *myfs_raw_inode(struct super_block *sb, ino_t ino, struct buffer_head **bh)
{
        struct myfs_inode *p;
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        *bh = sb_bread(sb, MYFS_INODE_BLOCK);
        if (!*bh) {
                printk("Unable to read inode block\n");
                return NULL;
        }
        p = (struct myfs_inode *)(*bh)->b_data;
        return p + ino;
}

struct inode *myfs_iget(struct super_block *sb, unsigned long ino)
{
        struct inode *inode;
        struct buffer_head *bh;
        struct myfs_inode *raw_inode;
        struct myfs_inode_info *myfs_inode;
        int i;
#if DEBUG
	printk("MYFS : %s\n", __func__);
#endif

        inode = iget_locked(sb, ino);
        if (!inode)
                return ERR_PTR(-ENOMEM);
        if (!(inode->i_state & I_NEW))
                return inode;

        raw_inode = myfs_raw_inode(sb, ino, &bh);
        if (!raw_inode) {
                iget_failed(inode);
                return ERR_PTR(-EIO);
        }
        inode->i_mode = raw_inode->i_mode; 
        inode->i_size = raw_inode->i_size;
#if DEBUG
	printk("MYFS : raw_inode i_mode %d\n", raw_inode->i_mode);
	printk("MYFS : raw_inode i_size %d\n", raw_inode->i_size);
#endif

        myfs_inode = myfs_i(inode);
        for(i=0; i < MYFS_DIRECT_BLOCK_NUM; i++)
        {
                myfs_inode->direct_blocks[i] = raw_inode->i_direct_blocks[i];
#if DEBUG
		if(raw_inode->i_direct_blocks[i])
			printk("MYFS : raw_inode->i_direct_blocks[%d] = %d\n", i, raw_inode->i_direct_blocks[i]);
#endif
        }
        for(i=0; i < MYFS_INDIRECT_BLOCK_NUM; i++)
        {
                myfs_inode->indirect_blocks[i] = raw_inode->i_indirect_blocks[i];
        }
	myfs_inode->dir_last_idx = raw_inode->i_dir_last_idx;
#if DEBUG
	printk("MYFS : inode_info->dir_last_idx : %d\n", raw_inode->i_dir_last_idx);
#endif

        myfs_set_inode(inode);
        brelse(bh);
        unlock_new_inode(inode);

        return inode;
}
