#!/bin/bash
set -e

cd "$(dirname "$0")/nauty2_9_3"
OUTPUT_DIR="../trees/small"
OUTPUT_FILE="$OUTPUT_DIR/small_trees.txt"
mkdir -p "$OUTPUT_DIR"

EXPECTED_TREES=100

if [ -f "$OUTPUT_FILE" ]; then
    COUNT=$(grep -c '^Graph ' "$OUTPUT_FILE" 2>/dev/null || echo 0)
    if [ "$COUNT" -eq "$EXPECTED_TREES" ]; then
        echo "small_trees.txt already complete ($COUNT/$EXPECTED_TREES trees). Skipping."
        exit 0
    else
        echo "small_trees.txt incomplete or invalid ($COUNT/$EXPECTED_TREES trees). Regenerating..."
    fi
else
    echo "small_trees.txt not found. Generating..."
fi

rm -f "$OUTPUT_FILE" "$OUTPUT_DIR/my_random_800_tree.s6"
touch "$OUTPUT_FILE"

for i in 10 20 30 40 50 60 70 80 90 100; do
    ./genrang -t ${i} 10 "$OUTPUT_DIR/random_${i}_trees.s6"
    ./showg -e "$OUTPUT_DIR/random_${i}_trees.s6" >> "$OUTPUT_FILE"
done

FINAL_COUNT=$(grep -c '^Graph ' "$OUTPUT_FILE" 2>/dev/null || echo 0)
echo "small_trees.txt generated ($FINAL_COUNT/$EXPECTED_TREES trees)."