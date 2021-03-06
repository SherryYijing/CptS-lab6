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
MTABLE mtable[NMTABLE];

// An OS kernel usually defines a finite number of PROC structures in its data area to represent processes in the system
// Global PROC pointer running to point at the PROC that is currently executing
PROC   proc[NPROC], *running;      // PROC structures and current executing PROC
OFT    oft[NOFT];                  // opened file instance

char gpath[128]; // global for tokenized components
char *name[32];  // assume at most 32 components in pathname
int   n;         // number of component strings

int fd, dev;
int nblocks, ninodes, bmap, imap, inode_start;
int oft_index = 0;  // track which index for global oft
int readlinkbuf = 0;   // for readlink

MINODE *iget();

char *disk = "disk3.1";   // default device
// char *disk = "disk1";
//char *disk = "diskimage";

#include "util.c"
#include "rmdir.c"
#include "cd_ls_pwd.c"
#include "quit.c"
#include "mkdir_creat.c"
#include "alloc_dealloc.c"
#include "link_unlink.c"
#include "misc.c"
#include "symlink_readlink.c"
#include "open_close_lseek.c"
//#include "openDir_readDir.c"
//#include "read_write.c"
#include "read_cat.c"
#include "write_cp.c"
#include "mount_umount.c"
