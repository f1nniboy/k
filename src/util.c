#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "util.h"

const char *xgetenv(const char *var, const char *fallback) {
    char *env = getenv(var);

    return env ? env[0] ? env : fallback : fallback;
}

char *path_normalize(char *d) {
    if (d) {
        for (size_t i = 1, l = strlen(d);
             d[l - i] == '/';
             d[l - i] = 0, i++);
    }

    return d;
}

int mkdir_p(const char* d) {
    for (char* p = strchr(d + 1, '/'); p; p = strchr(p + 1, '/')) {
        *p = 0;
        int ret = mkdir(d, 0755);
        *p = '/';

        if (ret == -1 && errno != EEXIST) {
            err("failed to create directory '%s': %s", d, strerror(errno));
            return -1;
        }
    }

    return 0;
}

int run_cmd(const char *cmd) {
    pid_t pid = fork();

    if (pid == -1) {
        err("failed to fork");
        return -1;

    } else if (pid == 0) {
        execl("/bin/sh", "sh", "-c", cmd, 0);

    } else {
        int status;

        waitpid(pid, &status, 0);

        if (WEXITSTATUS(status)) {
            err("command '%s' exited non-zero", cmd);
            return -1;
        }
    }

    return 0;    
}

int is_dir(const char *path) {
   struct stat statbuf;

   if (stat(path, &statbuf) != 0) {
       err("failed to stat path '%s': %s", path, strerror(errno));
       return 0;
   }

   return S_ISDIR(statbuf.st_mode);
}

