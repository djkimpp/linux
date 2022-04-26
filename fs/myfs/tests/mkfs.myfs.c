#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <linux/types.h>

#include "../myfs_fs.h"
#define DEBUG 1
int main(int argc, char **argv)
{
        int fd;
        char buffer[MYFS_BLOCKSIZE];
        struct myfs_super_block msb;
        struct myfs_inode root_inode;
        struct myfs_dir_entry myfs_dot_dentry;
        struct myfs_dir_entry myfs_dotdot_dentry;
        int i;
#if DEBUG
	struct myfs_inode file_inode;
        struct myfs_dir_entry file_dentry;
#endif
        if (argc != 2) {
                fprintf(stderr, "Usage: %s block_device_name\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fd = open(argv[1], O_RDWR | O_EXCL);
	if (fd == 0) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	/* zero disk  */
	memset(buffer, 0,  MYFS_BLOCKSIZE);
	for (i = 0; i < 128; i++) {
		pwrite(fd, buffer, MYFS_BLOCKSIZE, MYFS_BLOCKSIZE*i);
        }

	/* initialize super block */
	memset(&msb, 0, sizeof(struct myfs_super_block));
	msb.s_magic = MYFS_MAGIC;
        pwrite(fd, &msb, sizeof(msb), MYFS_SUPER_BLOCK);

	/* initialize root inode */
	memset(&root_inode, 0, sizeof(struct myfs_inode));
	root_inode.i_mode = S_IFDIR | 0755;
	root_inode.i_size = MYFS_BLOCKSIZE;
	root_inode.i_direct_blocks[0] = MYFS_FIRST_DATA_BLOCK;
	root_inode.i_dir_last_idx = 2;
	pwrite(fd, &root_inode, sizeof(root_inode), (MYFS_INODE_BLOCK * MYFS_BLOCKSIZE) + (sizeof(struct myfs_inode) * MYFS_ROOT_INO));

	/* initialize root data block */
	memset(&myfs_dot_dentry, 0, sizeof(myfs_dot_dentry));
	strcpy(myfs_dot_dentry.name, ".");
	myfs_dot_dentry.d_inode = MYFS_ROOT_INO;
	pwrite(fd, &myfs_dot_dentry, sizeof(myfs_dot_dentry), MYFS_FIRST_DATA_BLOCK *MYFS_BLOCKSIZE);
	memset(&myfs_dotdot_dentry, 0, sizeof(myfs_dotdot_dentry));
	strcpy(myfs_dotdot_dentry.name, "..");
	myfs_dotdot_dentry.d_inode = MYFS_ROOT_INO;
	pwrite(fd, &myfs_dotdot_dentry, sizeof(myfs_dotdot_dentry), (MYFS_FIRST_DATA_BLOCK *MYFS_BLOCKSIZE) + sizeof(struct myfs_dir_entry));

        /* initialize i-bitmap */
        buffer[0] = 0b00000011;
        pwrite(fd, &buffer[0], 1, MYFS_I_BITMAP_BLOCK * MYFS_BLOCKSIZE);

        /* initialize d-bitamp */
        buffer[0] = 0b00011111;
        pwrite(fd, &buffer[0], 1, MYFS_D_BITMAP_BLOCK * MYFS_BLOCKSIZE);
	close(fd);

	return 0;
}
