

#include <sys/types.h>
#include <string.h>
#include <regex.h>
#include <stdio.h>
#include <locale.h>

int main(int argc, char** argv) {
    int ret;
    regex_t reg;
    regmatch_t matches[10];

    if (argc != 3) {
        fprintf(stderr, "Usage: %s regex string\n", argv[0]);
        return 1;
    }

    setlocale(LC_ALL, ""); /* Use system locale instead of default "C" */

    if ((ret = regcomp(&reg, argv[1], 0)) != 0) {
        char buf[256];
        regerror(ret, &reg, buf, sizeof(buf));
        fprintf(stderr, "regcomp() error (%d): %s\n", ret, buf);
        return 1;
    }

    if ((ret = regexec(&reg, argv[2], 10, matches, 0)) == 0) {
        unsigned int i;
        char buf[256];
        size_t size;
        for (i = 0; i < (sizeof(matches) / sizeof(regmatch_t)); i++) {
            if (matches[i].rm_so == -1) break;
            size = matches[i].rm_eo - matches[i].rm_so;
            if (size >= sizeof(buf)) {
                fprintf(stderr, "match (%d-%d) is too long (%lu)\n",
                        matches[i].rm_so, matches[i].rm_eo, size);
                continue;
            }
            buf[size] = '\0';
            printf("%d: %d-%d: '%s'\n", i, matches[i].rm_so, matches[i].rm_eo,
                   strncpy(buf, argv[2] + matches[i].rm_so, size));

        }
    }

    return 0;
}
