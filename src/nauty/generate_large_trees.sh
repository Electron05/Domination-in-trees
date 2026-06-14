cd unpacked/
mkdir -p ../graphs/large
> ../graphs/large/my_trees.txt
for size in 512 1024 2048 4096 8192 16384 32768 65536; do
    ./genrang -t ${size} 1000 ../graphs/large/my_random_${size}_trees.s6
    ./showg -e ../graphs/large/my_random_${size}_trees.s6 >> ../graphs/large/my_trees.txt
done