#!/bin/bash

set -eu

ZIMAGE_FILE=${1:-}

if [[ -z "$ZIMAGE_FILE" ]]; then
    echo "./zimage-version.sh zImage"
    exit 255
fi

./zimage-extract "$1" |strings|grep 'Linux version [0-9]'|cut -d ' ' -f 3