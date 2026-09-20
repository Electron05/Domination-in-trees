#!/bin/bash
set -e

cd "$(dirname "$0")/nauty2_9_3"
mkdir -p ../trees/small
touch ../trees/small/small_trees.txt
for i in 10 20 30 40 50 60 70 80 90 100; do
    ./genrang -t ${i} 10 ../trees/small/my_random_${i}_trees.s6
done
for i in 10 20 30 40 50 60 70 80 90 100; do
    ./showg -e ../trees/small/my_random_${i}_trees.s6 >> ../trees/small/small_trees.txt
done

./genrang -t 800 1 ../trees/small/my_random_800_tree.s6
./showg -e ../trees/small/my_random_800_tree.s6 >> ../trees/small/small_trees.txt