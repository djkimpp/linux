#include "myfs.h"

static struct inode *myfs_alloc_inode(struct super_block *sb)
{
        struct myfs_inode_info *inode_info;
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        inode_info = kzalloc(sizeof(struct myfs_inode_info), GFP_KERNEL);
        if (!inode_info)
                return NULL;
        inode_init_once(&inode_info->vfs_inode);
        return &inode_info->vfs_inode;
}

static void myfs_destroy_inode(struct inode *inode)
{
#if DEBUG
	printk("MYFS : %s\n",__func__);
	if(inode == NULL)
		printk("MYFS : %s inode is null!!!!!!",__func__);
	else
		printk("MYFS : inode number is %ld\n", inode->i_ino);
#endif

        kfree(container_of(inode, struct myfs_inode_info, vfs_inode));
}

static int myfs_write_inode(struct inode *inode, struct writeback_control *wbc)
{
        struct buffer_head * bh;
        struct myfs_inode * raw_inode;
        struct myfs_inode_info *myfs_inode = myfs_i(inode);
        int i;
#if DEBUG
	printk("MYFS : %s inode num %ld\n",__func__, inode->i_ino);
#endif

        raw_inode = myfs_raw_inode(inode->i_sb, inode->i_ino, &bh);
        if (!raw_inode)
                return -EIO;
        raw_inode->i_mode = inode->i_mode;
        raw_inode->i_size = inode->i_size;
        for (i = 0; i < MYFS_DIRECT_BLOCK_NUM; i++)
                raw_inode->i_direct_blocks[i] = myfs_inode->direct_blocks[i];
        for (i = 0; i < MYFS_INDIRECT_BLOCK_NUM; i++)
                raw_inode->i_indirect_blocks[i] = myfs_inode->indirect_blocks[i];
	raw_inode->i_dir_last_idx = myfs_inode->dir_last_idx;
#if DEBUG
	printk("MYFS : inode_info->dir_last_idx : %d\n", myfs_inode->dir_last_idx);
#endif
        mark_buffer_dirty(bh);
        brelse(bh);

        return 0;
}

static const struct super_operations myfs_sops = {
        .alloc_inode    = myfs_alloc_inode,
        .destroy_inode  = myfs_destroy_inode,
        .write_inode    = myfs_write_inode
};

static int myfs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct myfs_sb_info *sbi;
	struct buffer_head *bh;
	struct inode *root_inode;
	int ret = -EINVAL;
#if DEBUG
	struct myfs_super_block *msb;
	printk("MYFS : %s\n", __func__);
#endif
	sbi = kzalloc(sizeof(struct myfs_sb_info), GFP_KERNEL);
	if(!sbi) {
		return -ENOMEM;
        }
	sb->s_fs_info = sbi;

        bh = sb_bread(sb, MYFS_SUPER_BLOCK);
	if(!bh)
		goto out_bad_sb;
#if DEBUG
	msb = (struct myfs_super_block *)bh->b_data;
	printk("MYFS : magic number %llx\n", msb->s_magic);
#endif
	sb->s_blocksize = MYFS_BLOCKSIZE;
	sb->s_blocksize_bits = MYFS_BLOCKSIZE_BITS;
	sb->s_maxbytes = MYFS_MAX_FILESIZE;
	sb->s_magic = MYFS_MAGIC;	
	sb->s_op = &myfs_sops;

	// i-bitmap
	if(!(bh = sb_bread(sb, MYFS_I_BITMAP_BLOCK))) {
		goto out_bad_sb;
        }
	memcpy(sbi->i_bitmap, bh->b_data, MYFS_BLOCKSIZE);
#if DEBUG
	printk("MYFS : i_bitmap\n");
	printk("%d ",(int)sbi->i_bitmap[0]);
#endif
	brelse(bh);

	// d-bitmap
	if(!(bh = sb_bread(sb, MYFS_D_BITMAP_BLOCK))) {
		goto out_bad_sb;
        }
	memcpy(sbi->d_bitmap, bh->b_data, MYFS_BLOCKSIZE);
#if DEBUG

	printk("MYFS : d_bitmap\n");
	printk("%d ",(int)sbi->d_bitmap[0]);

#endif

	brelse(bh);

	// root inode
	root_inode = myfs_iget(sb, MYFS_ROOT_INO);
	if (IS_ERR(root_inode)) {
                ret = PTR_ERR(root_inode);
                goto out_no_root;
	}

        ret = -ENOMEM;
        sb->s_root = d_make_root(root_inode);
        if (!sb->s_root) 
                goto out_no_root;

	printk("MYFS : return 0\n");
	return 0;

out_no_root:
        iput(root_inode);
	printk("MYFS : get root inode failed\n");
	goto out;
out_bad_sb:
	printk("MYFS : unable to read superblock\n");
out:
	sb->s_fs_info = NULL;
	kfree(sbi);
#if DEBUG
	printk("MYFS : %s return %d\n", __func__, ret);
#endif
	return ret;
}

static struct dentry *myfs_mount(struct file_system_type *fs_type,
        int flags, const char *dev_name, void *data)
{
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif
        return mount_bdev(fs_type, flags, dev_name, data, myfs_fill_super);
}

static void kill_myfs_super(struct super_block *sb)
{
	struct buffer_head *bh;
	struct myfs_sb_info *sbi = (struct myfs_sb_info *)sb->s_fs_info;
	// i-bitmap
	if(!(bh = sb_bread(sb, MYFS_I_BITMAP_BLOCK))) {
		goto out_bad_sb;
        }
	memcpy(bh->b_data, sbi->i_bitmap, MYFS_BLOCKSIZE);
	mark_buffer_dirty(bh);
	brelse(bh);

	// d-bitmap
	if(!(bh = sb_bread(sb, MYFS_D_BITMAP_BLOCK))) {
		goto out_bad_sb;
        }
	memcpy(bh->b_data, sbi->d_bitmap, MYFS_BLOCKSIZE);
	
	mark_buffer_dirty(bh);
	brelse(bh);

out_bad_sb:
	kill_block_super(sb);
}

static struct file_system_type myfs_type = {
	.owner = THIS_MODULE,
	.name = "myfs",
	.kill_sb = kill_myfs_super,
	.mount = myfs_mount,
	.fs_flags = FS_REQUIRES_DEV,
};

static int __init init_myfs(void)
{
        return register_filesystem(&myfs_type);
}

static void __exit exit_myfs(void)
{
        unregister_filesystem(&myfs_type);
}

module_init(init_myfs)
module_exit(exit_myfs)
MODULE_LICENSE("GPL");
