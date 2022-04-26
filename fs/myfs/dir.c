#include <linux/buffer_head.h>

#include "myfs.h"

static int myfs_add_link(struct dentry *dentry, struct inode *inode)
{
        struct inode *dir = d_inode(dentry->d_parent);
        const char *name = dentry->d_name.name;
        struct super_block *sb = dir->i_sb;
        struct buffer_head *bh;
        struct myfs_inode_info *parent_inode_info = myfs_i(dir);
        struct myfs_dir_entry *de;
        int err = 0;
        int i;

#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        bh = sb_bread(sb, parent_inode_info->direct_blocks[0]);
#if DEBUG
	printk("MYFS : parent_inode_info->dir_last_idx : %d\n", parent_inode_info->dir_last_idx);
#endif

        for(i = 0; i < parent_inode_info->dir_last_idx; i++) {
                de = (struct myfs_dir_entry *)bh->b_data + i;
                if(de->d_inode == 0)
                        break;
        }

        if(i == MYFS_MAX_DIR_ENTRY_NUM) {
                err = -ENOSPC;
                goto out;
        }

	if(i == parent_inode_info->dir_last_idx) {
		parent_inode_info->dir_last_idx++;
		de++;
#if DEBUG
	printk("MYFS : parent_inode_info->dir_last_idx : %d\n", parent_inode_info->dir_last_idx);
#endif
	}
 
        de->d_inode = inode->i_ino;
        memcpy(de->name, name, MYFS_FILE_NAME_LEN);
	mark_inode_dirty(dir);
        mark_buffer_dirty(bh);

out:
        brelse(bh);
#if DEBUG
	printk("MYFS : %s return value is %d\n",__func__, err);
#endif
        return err;
}

static int myfs_create(struct user_namespace *mnt_userns, struct inode *dir,
                        struct dentry *dentry, umode_t mode, bool excl)
{
        int error;
        struct inode *inode;
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        inode = myfs_new_inode(dir, mode);

        if (inode) {
                myfs_set_inode(inode);
                error = myfs_add_link(dentry, inode);
                if (error)
                        goto err_add_link;
                d_instantiate(dentry, inode);
                mark_inode_dirty(inode);
        }

        return 0;

err_add_link:
        inode_dec_link_count(inode);
        iput(inode);
        return error;
}

static struct myfs_dir_entry *myfs_find_entry(struct dentry *dentry, struct buffer_head **res_bh)
{
        const char *name = dentry->d_name.name;
        struct inode *dir = d_inode(dentry->d_parent);
        struct myfs_inode_info *inode_info = myfs_i(dir);
        struct super_block * sb = dir->i_sb;
        struct myfs_dir_entry *de;
        int i;
        struct buffer_head *bh;
        *res_bh = NULL;
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        bh = sb_bread(sb, inode_info->direct_blocks[0]);
        if(!bh) {
                printk("could not read block\n");
                return NULL;
        }
        *res_bh = bh;
 #if DEBUG
	printk("MYFS : inode_info->dir_last_idx : %d\n", inode_info->dir_last_idx);
#endif
       
        for(i = 0; i < inode_info->dir_last_idx; i++)
        {
                de = (struct myfs_dir_entry *)bh->b_data + i;
                if(de->d_inode == 0)
                        continue;
                if(strcmp(de->name, name) == 0) {
#if DEBUG
			printk("Found entry %s on position : %d\n", name, i);
#endif			
                        return de;
		}
        }

        return NULL;
}

static struct dentry *myfs_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags)
{
        struct super_block *sb = dir->i_sb;
        struct myfs_dir_entry *de;
        struct buffer_head *bh = NULL;
        struct inode * inode = NULL;
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        dentry->d_op = sb->s_root->d_op;

        if (dentry->d_name.len > MYFS_FILE_NAME_LEN)
                return ERR_PTR(-ENAMETOOLONG);

        de = myfs_find_entry(dentry, &bh);
        if (de) {
                inode = myfs_iget(dir->i_sb, de->d_inode);
        }

        d_add(dentry, inode);
        brelse(bh);

        return NULL;
}

static int myfs_make_empty(struct inode *inode, struct inode *dir)
{
        struct super_block *sb = inode->i_sb;
        struct buffer_head *bh;
        struct myfs_inode_info *inode_info = myfs_i(inode);
        struct myfs_dir_entry *de;

#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        inode_info->direct_blocks[0] = get_free_block(sb);
        inode->i_size = MYFS_BLOCKSIZE;
        inode->i_blocks += 1;
#if DEBUG
	printk("MYFS : %s get free block num %d\n",__func__, inode_info->direct_blocks[0]);
#endif

        bh = sb_bread(sb, inode_info->direct_blocks[0]);
        de = (struct myfs_dir_entry *)bh->b_data;

        de->d_inode = inode->i_ino;
        strcpy(de->name, ".");
        de++;
        de->d_inode = dir->i_ino;
        strcpy(de->name, "..");

	inode_info->dir_last_idx = 2;

        mark_buffer_dirty(bh);
        brelse(bh);

        return 0;
}

static int myfs_mkdir(struct user_namespace *mnt_userns, struct inode *dir,
                       struct dentry *dentry, umode_t mode)
{
        struct inode * inode;
        int err;
#if DEBUG
  	printk("MYFS : %s\n",__func__);
#endif
        inode = myfs_new_inode(dir, S_IFDIR | mode);
        if (!inode) {
                 err = -ENOSPC;
                 goto out;
        }

        myfs_set_inode(inode);

        err = myfs_make_empty(inode, dir);

        err = myfs_add_link(dentry, inode);
        if (err) {
                inode_dec_link_count(inode);
                goto out;
        }

        d_instantiate(dentry, inode);
	mark_inode_dirty(inode);
        inc_nlink(dir);

	return 0;
out:
        iput(inode);
        return err;
}

static bool myfs_empty_dir(struct inode * inode)
{
        struct super_block *sb = inode->i_sb;
        struct buffer_head *bh;
        struct myfs_inode_info *inode_info = myfs_i(inode);
        struct myfs_dir_entry *de;
        int ret = true;
        int i;
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        bh = sb_bread(sb, inode_info->direct_blocks[0]);
        if(!bh)
                return false;

        de = (struct myfs_dir_entry *)bh->b_data;
#if DEBUG
	printk("MYFS : inode_info->dir_last_idx : %d\n", inode_info->dir_last_idx);
#endif

        for(i = 0; i < inode_info->dir_last_idx; i++)
        {
                de += i;
                if ((de->d_inode != 0) &&
                   (strcmp(de->name, ".") != 0) &&
                   (strcmp(de->name, "..") != 0)) {
                        ret = false;
                        break;
                }
        }

        brelse(bh);
        return ret;
}

static int myfs_delete_dir_entry(struct inode *inode)
{
        int err;
	int i;
        int block;
        struct super_block *sb = inode->i_sb;
        struct myfs_inode_info *inode_info = myfs_i(inode);
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        for(i = 0; i < MYFS_MAX_FILEBLOCKS; i++)
        {
                block = get_block_location(inode, i);
                if(block == 0)
                        break;
                err = put_block(sb, block);
                if(err)
                        goto out;
        }

        for(i = 0; i < MYFS_INDIRECT_BLOCK_NUM; i++)
        {
		if(inode_info->indirect_blocks[i] != 0) {
	                err = put_block(sb, inode_info->indirect_blocks[i]);
                	if(err)
                        	goto out;
		}
        }

        err = put_inode(sb, inode->i_ino);
        if(err)
                goto out;

        inode_dec_link_count(inode);
#if DEBUG
	printk("MYFS : myfs_delete_dir_entry return value is %d\n", err);
#endif
       
out:
        return err;

}

static int myfs_unlink(struct inode *dir, struct dentry *dentry)
{
        int err = -ENOENT;
        struct myfs_dir_entry *de;
        struct buffer_head *bh;
        struct myfs_inode_info *inode_info = myfs_i(dir);
	struct super_block *sb = dir->i_sb;
	int i;
        const char *name = dentry->d_name.name;
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        bh = sb_bread(sb, inode_info->direct_blocks[0]);
        if(!bh)
		goto end_unlink;
#if DEBUG
	printk("MYFS : dir inode num : %ld inode_info->dir_last_idx : %d\n", dir->i_ino, inode_info->dir_last_idx);
#endif

        for(i = 0; i < inode_info->dir_last_idx; i++)
        {
                de = (struct myfs_dir_entry *)bh->b_data + i;
                if(de->d_inode == 0)
                        continue;
                if(strcmp(de->name, name) == 0) {
#if DEBUG
	printk("MYFS : de->name : %s de->d_inode: %d, i : %d\n", de->name, de->d_inode, i);
#endif
                        de->d_inode = 0;
                        break;
		}
        }

	if(i == inode_info->dir_last_idx)
		goto end_unlink;
#if DEBUG
	printk("i : %d dir_last_idx-1 : %d\n", i, inode_info->dir_last_idx-1);
#endif

	if(i == inode_info->dir_last_idx - 1) {
		inode_info->dir_last_idx -= 1;
#if DEBUG
	printk("MYFS : this is last idx ! change dir_last_idx : %d\n", inode_info->dir_last_idx);
#endif
	}

        mark_buffer_dirty(bh);
        brelse(bh);

        err = myfs_delete_dir_entry(dentry->d_inode);
	mark_inode_dirty(dir);

end_unlink:
#if DEBUG
	printk("MYFS : myfs_unlink return value is %d\n", err);
#endif

        return err;
}


static int myfs_rmdir(struct inode * dir, struct dentry *dentry)
{
        struct inode * inode = d_inode(dentry);
        int err = -ENOTEMPTY;
#if DEBUG
	printk("MYFS : %s\n inode num %ld\n",__func__, inode->i_ino);
#endif

        if (myfs_empty_dir(inode)) {
                err = myfs_unlink(dir, dentry);
                if (!err) {
                        inode_dec_link_count(dir);
                        inode_dec_link_count(inode);
                }
        }
#if DEBUG
	printk("MYFS : %s\n return value %d\n",__func__, err);
#endif

        return err;
}

static void myfs_set_link(struct myfs_dir_entry *de, struct inode *inode)
{
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        de->d_inode = inode->i_ino;
}

struct myfs_dir_entry * myfs_dotdot(struct inode *dir, struct buffer_head **p)
{
        struct super_block *sb = dir->i_sb;
        struct myfs_dir_entry *de = NULL;
        struct buffer_head *bh;
        struct myfs_inode_info *inode_info = myfs_i(dir);
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        bh = sb_bread(sb, inode_info->direct_blocks[0]);
        if(!bh)
                return NULL;
        *p = bh;
        de = (struct myfs_dir_entry *)bh->b_data;
        de++;
        return de;
}

static int myfs_rename(struct user_namespace *mnt_userns,
                        struct inode *old_dir, struct dentry *old_dentry,
                        struct inode *new_dir, struct dentry *new_dentry,
                        unsigned int flags)
{
        struct inode *old_inode = d_inode(old_dentry);
	struct myfs_inode_info *old_inode_info = myfs_i(old_inode);
	struct myfs_dir_entry *tmp_de;
        struct inode *new_inode = d_inode(new_dentry);
        struct buffer_head *dir_bh = NULL;
        struct myfs_dir_entry *dir_de = NULL;
        struct buffer_head *old_bh;
        struct myfs_dir_entry *old_de;
        int err = -ENOENT;
#if DEBUG
	printk("MYFS : %s\n",__func__);
#endif

        if (flags & ~RENAME_NOREPLACE)
                return -EINVAL;

        old_de = myfs_find_entry(old_dentry, &old_bh);
        if (!old_de)
                goto out;

        if (S_ISDIR(old_inode->i_mode)) {
                err = -EIO;
                dir_de = myfs_dotdot(old_inode, &dir_bh);
                if (!dir_de)
                        goto out_old;
        }

        if (new_inode) {
                struct buffer_head *bh;
                struct myfs_dir_entry *new_de;

                err = -ENOTEMPTY;
                if (dir_de && !myfs_empty_dir(new_inode))
                        goto out_dir;

                err = -ENOENT;
                new_de = myfs_find_entry(new_dentry, &bh);
                if (!new_de)
                        goto out_dir;
                myfs_set_link(new_de, old_inode);
                new_inode->i_ctime = current_time(new_inode);
                if (dir_de)
                        drop_nlink(new_inode);
                inode_dec_link_count(new_inode);
        } else {
                err = myfs_add_link(new_dentry, old_inode);
                if (err)
                        goto out_dir;
                if (dir_de)
                        inode_inc_link_count(new_dir);
        }

        old_de->d_inode = 0;
	tmp_de =(struct myfs_dir_entry *)old_bh->b_data + old_inode_info->dir_last_idx-1;
	if (tmp_de == old_de)
		old_inode_info->dir_last_idx--;	
        mark_inode_dirty(old_inode);

        if (dir_de) {
                myfs_set_link(dir_de, new_dir);
                inode_dec_link_count(old_dir);
        }
        return 0;

out_dir:
        if (dir_de) {
                mark_buffer_dirty(dir_bh);
                brelse(dir_bh);
        }
out_old:
        mark_buffer_dirty(old_bh);
        brelse(old_bh);
out:
        return err;
}

static int myfs_readdir(struct file *filp, struct dir_context *ctx)
{
        struct buffer_head *bh;
        struct myfs_dir_entry *de;
        struct inode *inode = filp->f_inode;
        struct myfs_inode_info *inode_info = myfs_i(inode);
        struct super_block *sb = inode->i_sb;
	int over;
#if DEBUG
	printk("MYFS : %s is called", __func__);
	printk("inode number : %ld\n", inode->i_ino);
	printk("data block number : %d\n", inode_info->direct_blocks[0]);
#endif

        bh = sb_bread(sb, inode_info->direct_blocks[0]);
        if(!bh) {
		printk("Could not read block\n");
                return -ENOMEM;
	}
#if DEBUG
	printk("MYFS : inode_info->dir_last_idx : %d\n", inode_info->dir_last_idx);
#endif
        for(; ctx->pos < inode_info->dir_last_idx; ctx->pos++)
        {
                de = (struct myfs_dir_entry *)bh->b_data + ctx->pos;
                if(de->d_inode == 0)
                        continue;
#if DEBUG
		printk("MYFS(%s) inode : %d, name : %s, ctx->pos : %lld\n", __func__, de->d_inode, de->name, ctx->pos);
#endif
                over = dir_emit(ctx, de->name, MYFS_FILE_NAME_LEN, de->d_inode, DT_UNKNOWN);
		if (over) {
			printk("Read %s from folder %s, ctx->pos : %lld\n",
				de->name,
				filp->f_path.dentry->d_name.name,
				ctx->pos);
			ctx->pos++;
			break;
		}
        }

        brelse(bh);
#if DEBUG
	printk("MYFS : %s return success", __func__);
#endif

        return 0;
}

const struct file_operations myfs_dir_operations = {
        .llseek         = generic_file_llseek,
        .read           = generic_read_dir,
        .iterate        = myfs_readdir,
        .fsync          = generic_file_fsync,
};

const struct inode_operations myfs_dir_inode_operations = {
        .create         = myfs_create,
        .lookup         = myfs_lookup,
        .mkdir          = myfs_mkdir,
        .rmdir          = myfs_rmdir,
        .rename         = myfs_rename,
        .unlink         = myfs_unlink,
};
