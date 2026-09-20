#!/bin/bash
set -e

cd "$(dirname "$0")/nauty2_9_3"
OUTPUT_DIR="../trees/large"
OUTPUT_FILE="$OUTPUT_DIR/large_trees.txt"
mkdir -p "$OUTPUT_DIR"

SIZES="512 1024 2048 4096 8192 16384"
EXPECTED_TREES=6000

if [ "$GENERATE_HUGE_TREES" = "true" ]; then
    SIZES="$SIZES 32768 65536"
    EXPECTED_TREES=8000
fi

if [ -f "$OUTPUT_FILE" ]; then
    COUNT=$(grep -c '^Graph ' "$OUTPUT_FILE" 2>/dev/null || echo 0)
    if [ "$COUNT" -eq "$EXPECTED_TREES" ]; then
        echo "large_trees.txt already complete ($COUNT/$EXPECTED_TREES trees). Skipping."
        exit 0
    else
        echo "large_trees.txt incomplete or invalid ($COUNT/$EXPECTED_TREES trees). Regenerating..."
    fi
else
    echo "large_trees.txt not found. Generating..."
fi

rm -f "$OUTPUT_FILE"
touch "$OUTPUT_FILE"

for size in $SIZES; do
    echo " - Generating 1000 trees of size $size..."
    ./genrang -t ${size} 1000 "$OUTPUT_DIR/random_${size}_trees.s6"
    ./showg -e "$OUTPUT_DIR/random_${size}_trees.s6" >> "$OUTPUT_FILE"
done

echo "large_trees.txt generated ($EXPECTED_TREES trees)."