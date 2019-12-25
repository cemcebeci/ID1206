#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

int main( int argc, char *argv[]) {
    if( argc < 2) {
        perror("usage: myls <dir>\n");
        return -1;
    }

    char *path = argv[1];
    DIR *dirp = opendir(path);

    struct dirent *entry;

    while((entry = readdir(dirp)) != NULL) {

        struct stat file_st;
        fstatat(dirfd(dirp), entry->d_name, &file_st, 0);

        switch( entry->d_type) {
            case DT_BLK :
                printf("b:");
                break;
            case DT_CHR :
                printf("c:");
                break;
            case DT_DIR :
                printf("d:");
                break;
            case DT_LNK :
                printf("l:");
                break;
            case DT_REG :
                printf("-:");
                break;
            case DT_SOCK :
                printf("s:");
                break;
            case DT_UNKNOWN :
                printf("u:");
                break;
        }
        //printf("type: %u", entry->d_type);
        printf("\tinode %lu", entry->d_ino);
        printf("\tsize %lu", file_st.st_size);
        printf("\tname: %s\n", entry->d_name);
    }

    
}