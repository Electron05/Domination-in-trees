#!/bin/bash
set -e

cd "$(dirname "$0")/nauty2_9_3"

OUTPUT_DIR="../generated_trees"
OUTPUT_FILE="$OUTPUT_DIR/generated_trees.txt"
PREVIEWS_FILE="$OUTPUT_DIR/preview_trees.txt"
CONFIG_FILE="$OUTPUT_DIR/.generated_trees.config"

mkdir -p "$OUTPUT_DIR"

if [ -z "$TREE_SIZES" ]; then
    echo "Error: TREE_SIZES is not set"
    exit 1
fi

if [ -z "$TREES_PER_BATCH" ]; then
    echo "Error: TREES_PER_BATCH is not set"
    exit 1
fi

PREVIEWS_PER_VARIANT="${PREVIEWS_PER_VARIANT:-0}"
PREVIEWS_SIZE="${PREVIEWS_SIZE:-0}"

SIZES="$TREE_SIZES"
NUM_BATCHES=$(echo $SIZES | wc -w)

EXPECTED_TREES=$((NUM_BATCHES * TREES_PER_BATCH))
CURRENT_CONFIG="SIZES=$SIZES;COUNT=$TREES_PER_BATCH;PREVIEWS=$PREVIEWS_PER_VARIANT,$PREVIEWS_SIZE"

if [ -f "$OUTPUT_FILE" ] && [ -f "$CONFIG_FILE" ]; then
    OLD_CONFIG=$(cat "$CONFIG_FILE")
    if [ "$OLD_CONFIG" = "$CURRENT_CONFIG" ]; then
        COUNT=$(grep -c '^Graph ' "$OUTPUT_FILE" || true)
        if [ "$COUNT" -eq "$EXPECTED_TREES" ]; then
            if [ "$PREVIEWS_PER_VARIANT" -eq 0 ] || [ -f "$PREVIEWS_FILE" ]; then
                echo "Trees already generated ($COUNT trees). Skipping."
                exit 0
            fi
        fi
    fi
fi

rm -f "$OUTPUT_FILE" "$PREVIEWS_FILE"
rm -f "$OUTPUT_DIR"/random_*_trees.s6

for size in $SIZES; do
    echo "Generating $TREES_PER_BATCH trees of size $size..."
    TMP_FILE="$OUTPUT_DIR/random_${size}_trees.s6"
    
    ./genrang -t $size $TREES_PER_BATCH "$TMP_FILE"
    ./showg -e "$TMP_FILE" >> "$OUTPUT_FILE"
    
    rm -f "$TMP_FILE"
done

if [ "$PREVIEWS_PER_VARIANT" -gt 0 ] && [ "$PREVIEWS_SIZE" -gt 0 ]; then
    echo "Generating $PREVIEWS_PER_VARIANT preview trees of size $PREVIEWS_SIZE..."
    TMP_PREVIEW="$OUTPUT_DIR/preview_temp.s6"
    ./genrang -t $PREVIEWS_SIZE $PREVIEWS_PER_VARIANT "$TMP_PREVIEW"
    ./showg -e "$TMP_PREVIEW" > "$PREVIEWS_FILE"
    rm -f "$TMP_PREVIEW"
fi

echo "$CURRENT_CONFIG" > "$CONFIG_FILE"
echo "Done. Generated $EXPECTED_TREES trees in $OUTPUT_FILE"