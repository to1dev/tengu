#!/bin/bash

# Default values
DEFAULT_VERSION="0.0.1-alpha"
DEFAULT_BINARY="tengu.exe"
DEFAULT_OUTPUT_DIR="releases"
VERSION="$DEFAULT_VERSION"
BINARY="$DEFAULT_BINARY"
OUTPUT_DIR="$DEFAULT_OUTPUT_DIR"
PLATFORM="windows-x64"

# Display help message
show_help() {
    echo "Usage: $0 [-v version] [-b binary] [-o output_dir] [-h]"
    echo ""
    echo "Options:"
    echo "  -v <version>     Specify version number (default: $DEFAULT_VERSION)"
    echo "  -b <binary>      Specify binary filename or path (default: $DEFAULT_BINARY)"
    echo "  -o <output_dir>  Specify output directory for generated files (default: $DEFAULT_OUTPUT_DIR)"
    echo "  -h               Show this help message"
    echo ""
    echo "Example:"
    echo "  $0 -v 1.0.0-beta -b path/to/myapp.exe -o dist"
    exit 1
}

# Parse command-line arguments
while getopts "v:b:o:h" opt; do
    case $opt in
        v) VERSION="$OPTARG";;
        b) BINARY="$OPTARG";;
        o) OUTPUT_DIR="$OPTARG";;
        h) show_help;;
        \?) echo "Error: Invalid option: -$OPTARG" >&2; show_help;;
    esac
done

# Construct filenames with output directory
ZIP_FILE="$OUTPUT_DIR/Tengu-${VERSION}-${PLATFORM}.zip"
CHECKSUM_FILE="$OUTPUT_DIR/Tengu-${VERSION}-shasums.txt"

# Check if binary file exists
if [ ! -f "$BINARY" ]; then
    echo "Error: $BINARY does not exist" >&2
    show_help
fi

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"
if [ $? -ne 0 ]; then
    echo "Error: Failed to create output directory $OUTPUT_DIR" >&2
    show_help
fi

# Remove existing ZIP file if it exists
if [ -f "$ZIP_FILE" ]; then
    rm "$ZIP_FILE"
    if [ $? -ne 0 ]; then
        echo "Error: Failed to remove existing $ZIP_FILE" >&2
        show_help
    fi
fi

# Create ZIP archive, junking paths to place binary at root
zip -j "$ZIP_FILE" "$BINARY"
if [ $? -ne 0 ]; then
    echo "Error: Failed to create ZIP archive" >&2
    show_help
fi

# Generate SHA256 checksum for ZIP file
sha256sum "$ZIP_FILE" > "$CHECKSUM_FILE"
if [ $? -ne 0 ]; then
    echo "Error: Failed to generate SHA256 checksum" >&2
    show_help
fi

# Sign checksum file with GPG
gpg --armor --detach-sign "$CHECKSUM_FILE"
if [ $? -ne 0 ]; then
    echo "Error: GPG signing failed" >&2
    show_help
fi

echo "Successfully generated $ZIP_FILE, $CHECKSUM_FILE, and its GPG signature ($CHECKSUM_FILE.asc)"
