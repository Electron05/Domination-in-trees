cd unpacked/
for i in {10..100..10}; do ./genrang -t ${i} 10 ../graphs/my_random_${i}_trees.s6; done
for i in {10..100..10}; do ./showg -e ../graphs/my_random_${i}_trees.s6 >> ../graphs/my_trees.txt; done