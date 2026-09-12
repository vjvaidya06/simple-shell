#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
 * match_pattern: checks if a filename matches a glob pattern with a single '*'
 * pattern: e.g. "foo*bar", "*.txt", "*", "foo*"
 * name: the filename to test (no path component)
 * returns 1 if match, 0 if not
 */
static int match_pattern(const char *pattern, const char *name) {
    const char *star = strchr(pattern, '*');
    if (!star) {
        // No wildcard — exact match only
        return strcmp(pattern, name) == 0;
    }

    size_t prefix_len = star - pattern;       // chars before '*'
    const char *suffix = star + 1;            // chars after '*'
    size_t suffix_len = strlen(suffix);
    size_t name_len   = strlen(name);

    // name must be at least prefix_len + suffix_len chars long
    if (name_len < prefix_len + suffix_len) return 0;

    // prefix must match the start of name
    if (strncmp(pattern, name, prefix_len) != 0) return 0;

    // suffix must match the end of name
    if (strcmp(name + name_len - suffix_len, suffix) != 0) return 0;

    return 1;
}

/*
 * expand_wildcard: expands a glob token into matching filenames.
 *
 * token:    the raw token, e.g. "*.txt", "baz/foo*bar", "foo*"
 * returns:  NULL-terminated array of heap-allocated strings (sorted),
 *           or NULL on error.
 *           If no files match, returns a 1-element array containing
 *           a copy of the original token (per spec).
 *
 * Caller is responsible for freeing each string and the array itself.
 */
static int cmp_str(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

char **expand_wildcard(const char *token, int *sz) {
    // --- Split token into directory and pattern ---
    const char *slash = strrchr(token, '/');
    char dir_path[4096];
    const char *pattern;

    if (slash) {
        size_t dir_len = slash - token;
        strncpy(dir_path, token, dir_len);
        dir_path[dir_len] = '\0';
        pattern = slash + 1;
    } else {
        strcpy(dir_path, ".");
        pattern = token;
    }

    // --- Open the directory ---
    DIR *dir = opendir(dir_path);
    if (!dir) {
        // Can't open dir: return token unchanged (same as no-match behavior)
        char **result = malloc(2 * sizeof(char *));
        if (!result) return NULL;
        result[0] = strdup(token);
        result[1] = NULL;
        return result;
    }

    // --- Collect matches ---
    char **matches = NULL;
    size_t count = 0;
    size_t capacity = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        const char *name = entry->d_name;

        // Skip hidden files (names starting with '.')
        if (name[0] == '.') continue;

        if (!match_pattern(pattern, name)) continue;

        // Grow the array if needed
        if (count + 1 > capacity) {
            capacity = capacity ? capacity * 2 : 8;
            char **tmp = realloc(matches, capacity * sizeof(char *));
            if (!tmp) {
                for (size_t i = 0; i < count; i++) free(matches[i]);
                free(matches);
                closedir(dir);
                return NULL;
            }
            matches = tmp;
        }

        // Build the full path if the token had a directory component
        char full[4096];
        if (slash) {
            snprintf(full, sizeof(full), "%s/%s", dir_path, name);
        } else {
            strncpy(full, name, sizeof(full) - 1);
            full[sizeof(full) - 1] = '\0';
        }

        matches[count] = strdup(full);
        if (!matches[count]) {
            for (size_t i = 0; i < count; i++) free(matches[i]);
            free(matches);
            closedir(dir);
            return NULL;
        }
        count++;
    }
    closedir(dir);

    // --- No matches: return the original token unchanged (per spec) ---
    if (count == 0) {
        free(matches);
        char **result = malloc(2 * sizeof(char *));
        if (!result) return NULL;
        result[0] = strdup(token);
        result[1] = NULL;
        return result;
    }

    // --- Sort results (readdir order is arbitrary) ---
    qsort(matches, count, sizeof(char *), cmp_str);

    // --- NULL-terminate ---
    char **tmp = realloc(matches, (count + 1) * sizeof(char *));
    if (!tmp) {
        for (size_t i = 0; i < count; i++) free(matches[i]);
        free(matches);
        return NULL;
    }
    matches = tmp;
    matches[count] = NULL;

    return matches;
}
