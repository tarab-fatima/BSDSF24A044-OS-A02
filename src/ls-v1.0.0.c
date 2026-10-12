#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define PATH_LEN 4096

void do_ls(const char *dir, int long_flag);
void show_long(const char *dir, const char *name);
void print_name(const char *dir, const char *name);
void mode_to_str(mode_t m, char *s);

int main(int argc, char *argv[])
{
    int long_flag = 0, opt;

    while ((opt = getopt(argc, argv, "l")) != -1) {
        if (opt == 'l')
            long_flag = 1;
        else {
            fprintf(stderr, "Usage: %s [-l] [dir...]\n", argv[0]);
            exit(1);
        }
    }

    if (optind == argc)
        do_ls(".", long_flag);
    else {
        for (int i = optind; i < argc; i++) {
            if (argc - optind > 1)
                printf("%s:\n", argv[i]);
            do_ls(argv[i], long_flag);
        }
    }
    return 0;
}

void do_ls(const char *dir, int long_flag)
{
    DIR *dp = opendir(dir);
    if (dp == NULL) {
        fprintf(stderr, "Cannot open directory:%s\n", dir);
        return;
    }
    struct dirent *entry;

    if (long_flag) {                          /* first pass: "total" line */
        long total = 0;
        struct stat st;
        char path[PATH_LEN];
        while ((entry = readdir(dp)) != NULL) {
            if (entry->d_name[0] == '.') continue;
            snprintf(path, PATH_LEN, "%s/%s", dir, entry->d_name);
            if (lstat(path, &st) == 0)
                total += st.st_blocks;
        }
        printf("total %ld\n", total / 2);
        rewinddir(dp);                        /* back to start for 2nd pass */
    }

    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;
        if (long_flag)
            show_long(dir, entry->d_name);
        else
            printf("%s\n", entry->d_name);
    }
    closedir(dp);
}

void show_long(const char *dir, const char *name)
{
    char path[PATH_LEN], perm[11];
    struct stat st;

    snprintf(path, PATH_LEN, "%s/%s", dir, name);
    if (lstat(path, &st) == -1) {
        perror("lstat");
        return;
    }

    mode_to_str(st.st_mode, perm);
    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);
    char *t = ctime(&st.st_mtime);            /* "Wed Jun 30 21:49:08 1993\n" */

    printf("%s %3ld ", perm, (long)st.st_nlink);
    printf("%-8s ", pw ? pw->pw_name : "?");
    printf("%-8s ", gr ? gr->gr_name : "?");
    printf("%8ld ", (long)st.st_size);
    printf("%.12s ", t + 4);                  /* "Jun 30 21:49" */
    print_name(dir, name);

    if (S_ISLNK(st.st_mode)) {
        char target[PATH_LEN];
        ssize_t n = readlink(path, target, PATH_LEN - 1);
        if (n != -1) {
            target[n] = '\0';
            printf(" -> %s", target);
        }
    }
    printf("\n");
}

void print_name(const char *dir, const char *name)
{
    (void)dir;
    printf("%s", name);
}

void mode_to_str(mode_t m, char *s)
{
    strcpy(s, "----------");

    if      (S_ISDIR(m))  s[0] = 'd';
    else if (S_ISLNK(m))  s[0] = 'l';
    else if (S_ISCHR(m))  s[0] = 'c';
    else if (S_ISBLK(m))  s[0] = 'b';
    else if (S_ISFIFO(m)) s[0] = 'p';
    else if (S_ISSOCK(m)) s[0] = 's';

    if (m & S_IRUSR) s[1] = 'r';
    if (m & S_IWUSR) s[2] = 'w';
    if (m & S_IXUSR) s[3] = 'x';
    if (m & S_IRGRP) s[4] = 'r';
    if (m & S_IWGRP) s[5] = 'w';
    if (m & S_IXGRP) s[6] = 'x';
    if (m & S_IROTH) s[7] = 'r';
    if (m & S_IWOTH) s[8] = 'w';
    if (m & S_IXOTH) s[9] = 'x';

    if (m & S_ISUID) s[3] = (m & S_IXUSR) ? 's' : 'S';
    if (m & S_ISGID) s[6] = (m & S_IXGRP) ? 's' : 'S';
    if (m & S_ISVTX) s[9] = (m & S_IXOTH) ? 't' : 'T';
}
