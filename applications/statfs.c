/*
 * print number of free blocks in filesystem.
 * Use: statfs <path>
 * Example:
 *   statfs /flash
 */


#include <rtthread.h>
#include <dfs.h>
#include <dfs_fs.h>
#ifdef RT_USING_DFS_V2
#include <dfs_mnt.h>
#endif

static int print_statfs(char *path)
{
    int32_t       retval = -RT_ERROR;
    struct statfs fs_stat;
    char         *fullpath = path;

#ifdef RT_USING_DFS_V1
    struct dfs_filesystem *fs = RT_NULL;

    fs = dfs_filesystem_lookup(path);
    if (fs != NULL)
        fullpath = fs->path;
#endif

#ifdef RT_USING_DFS_V2
    struct dfs_mnt *mnt = RT_NULL;

    mnt = dfs_mnt_lookup(path);
    if (mnt != NULL)
        fullpath = mnt->fullpath;
#endif

    retval = dfs_statfs(fullpath, &fs_stat);
    if (retval == 0)
    {
        uint32_t bsize  = fs_stat.f_bsize;
        uint32_t blocks = fs_stat.f_blocks;
        uint32_t bfree  = fs_stat.f_bfree;
        uint32_t bavail = fs_stat.f_bavail;
        rt_kprintf("path: %s block size: %u blocks: %u free: %u available: %u\r\n", fullpath, bsize, blocks, bfree, bavail);
    }
    else
        rt_kprintf("path: %s no stats\r\n", path);
    return retval;
}

static int cmd_statfs(int argc, char **argv)
{
    int32_t retval = -RT_ERROR;
    char   *path   = RT_NULL;

    if (argc < 2)
    {
        rt_kprintf("%s <path>\r\n", argv[0]);
    }
    else
    {
        print_statfs(argv[1]);
    }
    return 0;
}

MSH_CMD_EXPORT_ALIAS(cmd_statfs, statfs, statfs<path> filesystem stats);
