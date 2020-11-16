#include <unistd.h>
#include <sys/wait.h>

#include "arr.h"
#include "error.h"
#include "util.h"
#include "tar.h"

/**
 * Use libarchive for tar archives.
 */
#ifdef USE_LIBARCHIVE

static int tar_write_data(struct archive *ar, struct archive *aw) {
    const void *buf = 0;
    size_t l = 0;
    int64_t off = 0;

    for (int r = 0; ;) {
        if ((r = archive_read_data_block(ar, &buf, &l, &off)) == ARCHIVE_EOF) {
            break;
        }

        if (r != ARCHIVE_OK) {
            err("failed to extract archive: %s", archive_error_string(ar));
            return -1;
        }

        if ((r = archive_write_data_block(aw, buf, l, off)) != ARCHIVE_OK) {
            err("failed to extract archive: %s", archive_error_string(ar));
            return -1;
        }
    }

    return 0;
}

static int tar_strip_component(struct archive_entry *ent) {
    const char *path = archive_entry_pathname(ent);

    if (!path) {
        err("failed to allocate memory");
        return -1;
    }

    if (strchr(path, '/')) {
        path = strchr(path, '/');
    }

    if (*path == '/') {
        path++;
    }

    archive_entry_set_pathname(ent, path);
    return 0;
}

int tar_extract(const char *f, int flags) {
    struct archive *a = archive_read_new();

    if (!a) {
        return -1;
    }

    int ret = 0;
    struct archive *e = archive_write_disk_new();

    if (!e) {
        ret = -1;
        goto r_error;
    }

    archive_write_disk_set_options(e, flags);
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    if ((ret = archive_write_disk_set_standard_lookup(e)) != ARCHIVE_OK) {
        err("failed to extract archive: %s", archive_error_string(e));
        goto w_error;
    }

    if ((ret = archive_read_open_filename(a, f, 10240)) != ARCHIVE_OK) {
        err("failed to open archive: %s", archive_error_string(a));
        goto w_error;
    }

    for (struct archive_entry *ent = 0; ;) {
        if ((ret = archive_read_next_header(a, &ent)) == ARCHIVE_EOF) {
            ret = 0;
            break;
        }

        if (ret != ARCHIVE_OK) {
            err("failed to extract archive: %s", archive_error_string(a));
            break;
        }

        if ((ret = tar_strip_component(ent)) < 0) {
            break;
        }

        archive_write_header(e, ent);

        if ((ret = tar_write_data(a, e)) < 0) {
            break;
        }
    }

w_error:
    archive_write_close(e);
    archive_write_free(e);
r_error:
    archive_read_close(a);
    archive_read_free(a);

    return ret;
}

/**
 * Fallback to executing tar utility if libarchive not available.
 */
#else

int tar_extract(const char *f, int flags) {
    (void) flags;

    if (!f) {
        return -1;
    }

    pid_t pid = fork();

    if (pid == -1) {
        err_no("failed to fork");
        return -1;

    } else if (pid == 0) {
        /**
         * Requires a "smart" tar that is aware of compression and can
         * automagically deal with it (GNU tar, libarchive tar (BSDtar),
         * busybox tar, ...), The same applies to the use of
         * --strip-components 1.
         */
        execlp("tar", "tar", "xf", f, "--strip-components", "1", 0);

    } else {
        int status = 0;

        waitpid(pid, &status, 0);

        if (WEXITSTATUS(status)) {
            err_no("tar exited %d", status);
            return -1;
        }
    }

    return 0;
}

#endif
