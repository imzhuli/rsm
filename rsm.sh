#!/bin/bash
ulimit -n 999999
ulimit -c unlimited
#ulimit -a

PROGRAM="app_rsm"

PID=`ps aux | grep -v "/bin/sh" | grep -v "grep" | grep -i "$PROGRAM" | awk '{print $2}'`
if [ -n "$PID" ]; then
    echo "Found pid=" $PID
    echo $PID | xargs kill -9
else
    echo "Program not found"
fi

sleep 1
./build/bin/app_rsm -c sample/rsm.conf > /dev/null &
