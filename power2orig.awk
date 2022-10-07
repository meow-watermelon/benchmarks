#!/usr/bin/env -S awk -f

BEGIN {
    init = 1000
    start_time = systime()

    while (1) {
        if (length(init) % 2 != 0) {
            init *= 10
        }

        splitter = length(init) / 2

        fore = substr(init, 1, splitter)
        post = substr(init, splitter + 1, length(init))

        if ((fore + post) ^ 2 == init) {
            printf "%d %d %d %d\n", fore, post, init, systime() - start_time
        }

        init += 1
    }
}
