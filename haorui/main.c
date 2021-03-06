/****************************************************************************
*                   KCW testing ext2 file system                            *
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "type.h"

// globals
MINODE minode[NMINODE];
MINODE *root;

PROC   proc[NPROC], *running;

char gpath[128]; // global for tokenized components
char *name[32];  // assume at most 32 components in pathname
int   n;         // number of component strings

int fd, dev;
int nblocks, ninodes, bmap, imap, inode_start;

MINODE *iget();

#include "util.c"
#include "rmdir.c"
#include "cd_ls_pwd.c"
#include "mkdir_creat.c"
#include "alloc_dealloc.c"
#include "link_unlink.c"
#include "symlink_readlink.c"


int quit()  // write all modified minode to disk
{
  int i;
  MINODE *mip;
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount > 0)
      iput(mip);
  }
  exit(0);
}

int init()
{
  int i, j;
  MINODE *mip;
  PROC   *p;

  printf("init()\n");

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mptr = 0;
  }
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->uid = p->gid = 0;
    p->cwd = 0;
    p->status = FREE;
    for (j=0; j<NFD; j++)
      p->fd[j] = 0;
  }
}

// load root INODE and set root pointer to it (p 328)
int mount_root()  // mount root file system
{  
  printf("mount_root() dev=%d\n", dev);
  root = iget(dev, 2);
}

char *disk = "diskimage";
int main(int argc, char *argv[ ])
{
  char* readLinkBuf=malloc(BLKSIZE);
  int ino;
  char buf[BLKSIZE];
  char line[128], cmd[32], pathname[128], temp[128];

  if (argc > 1)
    disk = argv[1];

  printf("checking EXT2 FS ....");
  if ((fd = open(disk, O_RDWR)) < 0){
    printf("open %s failed\n", disk);
    exit(1);
  }
  dev = fd;   

  /********** read super block  ****************/
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;

  /* verify it's an ext2 file system ***********/
  if (sp->s_magic != 0xEF53){
      printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
      exit(1);
  }     
  printf("EXT2 FS OK\n");
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;

  get_block(dev, 2, buf); 
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  inode_start = gp->bg_inode_table;
  printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, inode_start);

  init();  
  mount_root();
  printf("root refCount = %d\n", root->refCount);

  printf("creating P0 as running process\n");
  running = &proc[0];
  running->status = READY;
  running->cwd = iget(dev, 2);
  printf("root refCount = %d\n", root->refCount);

  while(1){
    printf("input command : [ls|cd|pwd|mkdir|rmdir|creat|link|unlink|quit] ");
    fgets(line, 128, stdin);
    line[strlen(line)-1] = 0;

    if (line[0]==0)
       continue;
    pathname[0] = 0;

    sscanf(line, "%s %s %s", cmd, pathname, temp);
    printf("cmd=%s pathname=%s temp = %s\n", cmd, pathname, temp);
    printf("dev = %d\n", dev);
    if (strcmp(cmd, "ls")==0)
       ls(pathname);
    if (strcmp(cmd, "cd")==0)
       chdir(pathname);
    if (strcmp(cmd, "pwd")==0)
       pwd(running->cwd);
    if(strcmp(cmd, "mkdir") == 0)
      make_dir(pathname);
    if(strcmp(cmd, "creat") == 0)
      creat_file(pathname);
    if(strcmp(cmd, "rmdir")==0)
      rmdir(pathname);
    if(strcmp(cmd, "link") == 0)
    	link(pathname, temp);
    if(strcmp(cmd, "unlink") == 0)
    	unlink(pathname);
    if(strcmp(cmd, "symlink")==0)
      symlink(pathname, temp);
    if(strcmp(cmd, "readlink")==0)
      readlink(pathname, readLinkBuf);
    if (strcmp(cmd, "quit")==0)
      quit();
  }
}


