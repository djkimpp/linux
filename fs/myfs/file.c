#include "myfs.h"

static void myfs_truncate(struct inode *inode)
{
        struct super_block *sb = inode->i_sb;
        __u32 block_num;
        __u32 num;
        int i;
        int err;
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        block_truncate_page(inode->i_mapping, inode->i_size, myfs_get_block);

        block_num = (inode->i_size + sb->s_blocksize - 1) >> sb->s_blocksize_bits;
        num = get_block_location(inode, block_num);
        if(num == 0) {
                int new_block;
                for(i = block_num; i < MYFS_MAX_FILEBLOCKS; i++)
                {
                        err = allocate_new_block(inode, i, &new_block);
                        if(err)
                                break;
                }
        } else {
                for(i = MYFS_MAX_FILEBLOCKS - 1; i >= block_num; i--) {
                        err = deallocate_block(inode, i);
                        if(err)
                                return;
                }
        }

        mark_inode_dirty(inode);
}

static int myfs_setattr(struct user_namespace *mnt_userns,
                         struct dentry *dentry, struct iattr *attr)
{
        struct inode *inode = d_inode(dentry);
        int error;
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        error = setattr_prepare(&init_user_ns, dentry, attr);
        if (error)
                return error;

        if ((attr->ia_valid & ATTR_SIZE) &&
            attr->ia_size != i_size_read(inode)) {
                error = inode_newsize_ok(inode, attr->ia_size);
                if (error)
                        return error;

                truncate_setsize(inode, attr->ia_size);
                myfs_truncate(inode);
        }

        setattr_copy(&init_user_ns, inode, attr);
        mark_inode_dirty(inode);
        return 0;
}

int myfs_getattr(struct user_namespace *mnt_userns, const struct path *path,
                  struct kstat *stat, u32 request_mask, unsigned int flags)
{
        struct super_block *sb = path->dentry->d_sb;
        struct inode *inode = d_inode(path->dentry);
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        generic_fillattr(&init_user_ns, inode, stat);
        stat->blocks = (inode->i_size + 512 - 1) / 512;
        stat->blksize = sb->s_blocksize;
        return 0;
}

static int myfs_writepage(struct page *page, struct writeback_control *wbc)
{
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        return block_write_full_page(page, myfs_get_block, wbc);
}

static int myfs_readpage(struct file *file, struct page *page)
{
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        return block_read_full_page(page, myfs_get_block);
}

static int myfs_write_begin(struct file *file, struct address_space *mapping,
                        loff_t pos, unsigned len, unsigned flags,
                        struct page **pagep, void **fsdata)
{
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        return block_write_begin(mapping, pos, len, flags, pagep,
                                 myfs_get_block);
}

const struct inode_operations myfs_file_inode_operations = {
        .setattr        = myfs_setattr,
        .getattr        = myfs_getattr,
};

const struct file_operations myfs_file_operations = {
        .llseek         = generic_file_llseek,
        .read_iter      = generic_file_read_iter,
        .write_iter     = generic_file_write_iter,
        .mmap           = generic_file_mmap,
        .fsync          = generic_file_fsync,
};

const struct address_space_operations myfs_aops = {
        .readpage = myfs_readpage,
        .writepage = myfs_writepage,
        .write_begin = myfs_write_begin,
        .write_end = generic_write_end,
};

