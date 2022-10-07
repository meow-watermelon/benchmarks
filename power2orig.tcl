#!/usr/bin/env tclsh

set init 1000
set start_time [clock microseconds]

while {true} {
    if {[expr {[string length $init] % 2}] != 0} {
        set init [expr {$init * 10}]
    }

    set splitter [expr {[string length $init] / 2}]

    set fore [string range $init 0 $splitter-1]
    set post [string range $init $splitter end]

    if {[expr {([scan $fore %lld] + [scan $post %lld]) ** 2}] == $init} {
        puts "$fore $post $init [expr {([clock microseconds] - $start_time) / 1000000.0}]"
    }

    incr init
}
