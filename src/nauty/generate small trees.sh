cd unpacked/
touch ../graphs/small/my_trees.txt
for i in {10..100..10}; do ./genrang -t ${i} 10 ../graphs/small/my_random_${i}_trees.s6; done
for i in {10..100..10}; do ./showg -e ../graphs/small/my_random_${i}_trees.s6 >> ../graphs/small/my_trees.txt; done

./genrang -t 800 1 ../graphs/small/my_random_800_tree.s6
./showg -e ../graphs/small/my_random_800_tree.s6 >> ../graphs/small/my_trees.txt