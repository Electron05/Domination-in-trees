#!/bin/bash
set -e

cd "$(dirname "$0")/nauty2_9_3"
mkdir -p ../trees/large
touch ../trees/large/large_trees.txt

SIZES="512 1024 2048 4096 8192 16384"
if [ "$GENERATE_HUGE_TREES" = "true" ]; then
    SIZES="$SIZES 32768 65536"
fi

for size in $SIZES; do
    ./genrang -t ${size} 1000 ../trees/large/my_random_${size}_trees.s6
done
for size in $SIZES; do
    ./showg -e ../trees/large/my_random_${size}_trees.s6 >> ../trees/large/large_trees.txt
done