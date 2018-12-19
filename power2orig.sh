#!/bin/bash

init=1000
start_time=$(date +%s)

while true
do
    if [[ $((${#init} % 2)) -ne 0 ]]
    then
        init=$(($init * 10))
    fi

    splitter=$((${#init} / 2))
    fore=${init:0:$splitter}
    post=${init:$splitter:$((${#init} - 1))}

    if [[ $((($fore + $((10#$post))) ** 2)) -eq $init ]] 
    then
        echo "$fore $post $init $(($(date +%s) - $start_time))"
    fi

    init=$(($init + 1))
done
