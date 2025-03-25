#!/bin/bash

# Script to download token data from Jupiter Aggregator API
# Usage: ./download_tokens.sh [OPTIONS] [OUTPUT_PATH]

# Default variables
URL="https://api.jup.ag/tokens/v1/all"
DEFAULT_OUTPUT_FILE="tokens.json"
COUNT_TOKENS=false

# Display help info
function show_help {
    echo "Usage: $0 [OPTIONS] [OUTPUT_PATH]"
    echo ""
    echo "Downloads token data from Jupiter API"
    echo ""
    echo "Options:"
    echo "  -h, --help     Show this help message"
    echo "  -c, --count    Count the number of tokens (requires jq)"
    echo ""
    echo "Arguments:"
    echo "  OUTPUT_PATH    Optional: Path where tokens.json will be saved"
    echo "                 Default: Current directory ($DEFAULT_OUTPUT_FILE)"
    echo ""
    echo "Examples:"
    echo "  $0                     # Save as ./tokens.json"
    echo "  $0 /path/to/data/      # Save as /path/to/data/tokens.json"
    echo "  $0 --count             # Save and count tokens"
    echo ""
}

# Process command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -c|--count)
            COUNT_TOKENS=true
            shift
            ;;
        *)
            # Assume this is the output path
            OUTPUT_PATH="$1"
            shift
            ;;
    esac
done

# Determine output path and filename
if [ -z "$OUTPUT_PATH" ]; then
    # No argument provided, use default
    OUTPUT_FILE="$DEFAULT_OUTPUT_FILE"
    echo "Using default output path: $OUTPUT_FILE"
else
    # Check if the provided path is a directory
    if [ -d "$OUTPUT_PATH" ]; then
        # It's a directory, append default filename
        OUTPUT_FILE="${OUTPUT_PATH%/}/$DEFAULT_OUTPUT_FILE"
        echo "Saving to directory: $OUTPUT_FILE"
    else
        # It's a file path or doesn't exist
        OUTPUT_FILE="$OUTPUT_PATH"

        # Create parent directories if they don't exist
        OUTPUT_DIR=$(dirname "$OUTPUT_FILE")
        if [ ! -d "$OUTPUT_DIR" ] && [ "$OUTPUT_DIR" != "." ]; then
            echo "Creating directory: $OUTPUT_DIR"
            mkdir -p "$OUTPUT_DIR"
            if [ $? -ne 0 ]; then
                echo "Error: Failed to create directory: $OUTPUT_DIR"
                exit 1
            fi
        fi
    fi
fi

# Check if output location is writable
touch "$OUTPUT_FILE" 2>/dev/null
if [ $? -ne 0 ]; then
    echo "Error: Cannot write to $OUTPUT_FILE"
    echo "Please check permissions or provide a different path."
    exit 1
fi

echo "Downloading token data from Jupiter API..."

# Check if wget is installed
if ! command -v wget &> /dev/null; then
    echo "Error: wget is not installed. Please install wget and try again."
    exit 1
fi

# Try to download the file
if wget -O "$OUTPUT_FILE" "$URL"; then
    # Get file size
    FILE_SIZE=$(du -h "$OUTPUT_FILE" | cut -f1)
    echo "Download successful! Saved $FILE_SIZE file."

    # Count tokens only if requested
    if [ "$COUNT_TOKENS" = true ]; then
        if command -v jq &> /dev/null; then
            echo "Counting tokens (this may take a while for large files)..."
            TOKEN_COUNT=$(jq '. | length' "$OUTPUT_FILE")
            echo "Token count: $TOKEN_COUNT"
        else
            echo "Warning: jq is not installed. Cannot count tokens."
            echo "Install jq package to use the --count option."
        fi
    fi
else
    echo "Error: Failed to download token data."
    # Clean up partial download
    rm -f "$OUTPUT_FILE"
    exit 1
fi

echo "File saved as: $OUTPUT_FILE"
echo "You can now run your LMDB import program."

exit 0
