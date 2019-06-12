#!/bin/sh

if [ "$*" == "" ]; then
    echo usage: `basename $0` platform
    exit 0
fi


docker run --rm dockcross/$1 > dockcross-$1
chmod +x dockcross-$1
