#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "util.h"
#include "pkg.h"

void pkg_load(package **head, char *pkg_name) {
    package *new_pkg = (package*) malloc(sizeof(package));    
    package *last = *head;

    if (!new_pkg) {
        printf("error: Failed to allocate memory\n");
        exit(1);
    }

    new_pkg->next = NULL;
    new_pkg->name = pkg_name;
    new_pkg->path = pkg_find(pkg_name);

    if (!*head) {
        new_pkg->prev = NULL;
        *head = new_pkg;
        return;
    }

    while (last->next) {
        last = last->next;
    }

    last->next = new_pkg;
    new_pkg->prev = last;
}


struct version pkg_version(char *repo_dir) {
    struct version version = {0};
    FILE *file;
    char *buf = 0;

    chdir(repo_dir);
    file = fopen("version", "r");

    if (!file) {
        printf("error: version file does not exist\n");
        exit(1);
    }

    getline(&buf, &(size_t){0}, file);
    fclose(file);

    if (!buf) {
        printf("error: version file is incorrect\n");
        exit(1);
    }

    version.version = strtok(buf,    " 	\n");
    version.release = strtok(NULL,   " 	\n");

    if (!version.release) {
        printf("error: release field missing\n");
        exit(1);
    }

    chdir(PWD);

    return version;
}

char **pkg_find(char *pkg_name) {
   char **paths = NULL;
   int  n = 0;
   char cwd[PATH_MAX];
   char **repos = REPOS;

   while (*repos) {
       if (chdir(*repos) != 0) {
           printf("error: Repository not accessible\n");       
           exit(1);
       }

       if (chdir(pkg_name) == 0) {
           paths = realloc(paths, sizeof(char*) * ++n);

           if (paths == NULL) {
               printf("Failed to allocate memory\n");
               exit(1);
           }

           paths[n - 1] =  strdup(getcwd(cwd, sizeof(cwd)));
       }

       ++repos;
   }

   chdir(PWD);
   paths = realloc(paths, sizeof(char*) * (n + 1));
   paths[n] = 0;

   if (*paths) {
       return paths;

   } else {
       printf("error: %s not in any repository\n", pkg_name);
       exit(1);
   }
}

void pkg_list(char *pkg_name) {
    struct version version;
    char *db = "/var/db/kiss/installed"; 
    char *path;
    char cwd[PATH_MAX];

    if (chdir(db) != 0) {
        printf("error: Package db not accessible\n");
        exit(1);
    }

    if (chdir(pkg_name) != 0) {
        printf("error: Package %s not installed\n", pkg_name);
        exit(1);

    } else {
        path = getcwd(cwd, sizeof(cwd)); 
        version = pkg_version(path);
        printf("%s %s %s\n", pkg_name, version.version, version.release);
    }

    chdir(PWD);
}

int pkg_sources(package pkg) {
   char **repos = pkg_find(pkg.name); 
   char *dest, *source;
   FILE *file;
   size_t  lsiz=0;
   char*   lbuf=0;
   ssize_t llen=0;

   chdir(*repos);
   file = fopen("sources", "r");

   if (!file) {
       printf("error: Sources file invalid\n");
       exit(1);
   }

   while ((llen = getline(&lbuf, &lsiz, file)) > 0) {
       // Skip comments and blank lines.
       if ((lbuf)[0] == '#' || (lbuf)[0] == '\n') {
           continue;
       }

       source = strtok(lbuf, " 	\n");
       dest   = strtok(NULL, " 	\n");

       /* if (mkpath(src_dir) != 0) { */
       /*     printf("%s (%s)\n", pkg, "Couldn't create source directory"); */
       /*     exit(1); */
       /* } */

       /* source = strtok(lbuf, " 	"); */ 
       /* dest   = strjoin(src_dir, basename(source), "/"); */
       /* local  = strjoin(repo_dir, source, "/"); */

       printf("%s -> %s\n", source, dest);
       /* if (access(dest, F_OK) != -1) { */
       /*     printf("%s (Found cached source %s)\n", pkg, dest); */
        
       /* } else if (strncmp(source, "https://", 8) == 0 || */
       /*            strncmp(source, "http://",  7) == 0) { */
       /*     printf("%s (Downloading %s)\n", pkg, source); */
       /*     source_download(source); */

       /* } else if (access(local, F_OK) != -1) { */
       /*     printf("%s (Found local source %s)\n", pkg, local); */

       /* } else { */
       /*     printf("%s (No local file %s)\n", local); */
       /*     exit(1); */
       /* } */
   }

   fclose(file);
   return 0; 
}
