#!/bin/bash

# put folloing 4 lines into into /etc/rc.local
#GIP="/mnt/b/aps/"
#export GIP
#su chris -c "$GIP/gip.sh &"
#echo "$GIP started."

ulimit -v 600000

cd $GIP

until $GIP/gip >/dev/null; do
  echo "server failed $? ... respawning" >&2
  sleep 10
  if [ -e $GIP/gip-new ]; then
    mv $GIP/gip $GIP/gip-bak
    mv $GIP/gip-new $GIP/gip
    chmod a+x $GIP/gip
  fi
done

