#!/bin/bash
ulimit -n 999999
ulimit -c unlimited
#ulimit -a

PROGRAM="app_rsm"
SLEEPTIME=125

PID=`ps aux | grep -v "/bin/sh" | grep -v "grep" | grep -i "$PROGRAM" | awk '{print $2}'`
if [ -n "$PID" ]; then
    echo "Found pid=" $PID
    echo $PID | xargs kill -9
    echo "process killed, sleep ${SLEEPTIME}s to restart"
    sleep ${SLEEPTIME}
else
    echo "Program not found"
fi

./build/bin/app_rsm -c sample/rsm.conf > /dev/null &
