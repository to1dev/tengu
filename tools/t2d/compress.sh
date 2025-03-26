#!/bin/bash

# Advanced script to create a compressed archive of LMDB database for R2 upload
# Uses zstd compression for optimal compression ratio and speed

# Default settings
DEFAULT_DB_PATH="./lmdb_data"
DEFAULT_OUTPUT_NAME="tokens.tar.zst"
DEFAULT_COMPRESSION_LEVEL=10  # High compression level (1-19, higher = better compression but slower)

# Function to display usage information
show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo "Creates a compressed archive of an LMDB database for upload to R2."
    echo
    echo "Options:"
    echo "  -i, --input PATH       Path to LMDB database directory (default: $DEFAULT_DB_PATH)"
    echo "  -o, --output FILE      Output archive filename (default: $DEFAULT_OUTPUT_NAME)"
    echo "  -c, --compress LEVEL   Compression level (1-19, default: $DEFAULT_COMPRESSION_LEVEL)"
    echo "  -h, --help             Display this help message"
    echo
    echo "Examples:"
    echo "  $0 --input ./my_database --output my_archive.tar.zst"
    echo "  $0 --compress 10       # Faster compression but larger file"
    echo
}

# Function to check prerequisites
check_prerequisites() {
    if ! command -v zstd &> /dev/null; then
        echo "Error: zstd compression tool not found." >&2
        echo "Please install zstd first:" >&2
        echo "  - Debian/Ubuntu: sudo apt-get install -y zstd" >&2
        echo "  - CentOS/RHEL: sudo yum install -y zstd" >&2
        echo "  - macOS: brew install zstd" >&2
        return 1
    fi

    if ! command -v tar &> /dev/null; then
        echo "Error: tar archiving tool not found." >&2
        return 1
    fi

    return 0
}

# Parse command line arguments
DB_PATH="$DEFAULT_DB_PATH"
OUTPUT_FILE="$DEFAULT_OUTPUT_NAME"
COMPRESSION_LEVEL="$DEFAULT_COMPRESSION_LEVEL"

while [[ $# -gt 0 ]]; do
    case "$1" in
        -i|--input)
            DB_PATH="$2"
            shift 2
            ;;
        -o|--output)
            OUTPUT_FILE="$2"
            shift 2
            ;;
        -c|--compress)
            COMPRESSION_LEVEL="$2"
            shift 2
            ;;
        -h|--help)
            show_usage
            exit 0
            ;;
        *)
            echo "Error: Unknown option: $1" >&2
            show_usage
            exit 1
            ;;
    esac
done

# Validate inputs
if ! check_prerequisites; then
    exit 1
fi

if [ ! -d "$DB_PATH" ]; then
    echo "Error: Database directory not found: $DB_PATH" >&2
    echo "Please check the path and try again." >&2
    exit 1
fi

if [ ! -r "$DB_PATH" ]; then
    echo "Error: Cannot read database directory: $DB_PATH" >&2
    echo "Please check permissions and try again." >&2
    exit 1
fi

# Check if data.mdb exists (basic LMDB structure check)
if [ ! -f "$DB_PATH/data.mdb" ]; then
    echo "Warning: Directory may not be an LMDB database (data.mdb not found)" >&2
    echo "Are you sure this is the correct path?" >&2
    echo "Press Ctrl+C to abort or Enter to continue anyway..." >&2
    read -r
fi

# Validate compression level
if ! [[ "$COMPRESSION_LEVEL" =~ ^[0-9]+$ ]] || [ "$COMPRESSION_LEVEL" -lt 1 ] || [ "$COMPRESSION_LEVEL" -gt 19 ]; then
    echo "Error: Invalid compression level: $COMPRESSION_LEVEL" >&2
    echo "Compression level must be between 1 and 19" >&2
    exit 1
fi

# Check if output file already exists
if [ -e "$OUTPUT_FILE" ]; then
    echo "Warning: Output file already exists: $OUTPUT_FILE" >&2
    echo "Press Ctrl+C to abort or Enter to overwrite..." >&2
    read -r
fi

# Check if we have enough free space
DB_SIZE=$(du -sk "$DB_PATH" | cut -f1)
FREE_SPACE=$(df -k . | tail -1 | awk '{print $4}')

if [ "$DB_SIZE" -gt "$FREE_SPACE" ]; then
    echo "Error: Not enough free disk space to create archive" >&2
    echo "Database size: $(du -sh "$DB_PATH" | cut -f1)" >&2
    echo "Free space: $(df -h . | tail -1 | awk '{print $4}')" >&2
    exit 1
fi

# Create archive
echo "Creating compressed archive from: $DB_PATH"
echo "Output file: $OUTPUT_FILE"
echo "Compression level: $COMPRESSION_LEVEL"
echo "This may take a while depending on database size..."

if tar -cf - "$DB_PATH" | zstd -z "-$COMPRESSION_LEVEL" -T0 > "$OUTPUT_FILE"; then
    # Get size information for reporting
    ORIGINAL_SIZE=$(du -sh "$DB_PATH" | cut -f1)
    COMPRESSED_SIZE=$(du -h "$OUTPUT_FILE" | cut -f1)
    COMPRESSION_RATIO=$(echo "scale=2; $(du -sk "$DB_PATH" | cut -f1) / $(du -sk "$OUTPUT_FILE" | cut -f1)" | bc)

    echo "Success! Database successfully archived and compressed."
    echo "Original size: $ORIGINAL_SIZE"
    echo "Compressed size: $COMPRESSED_SIZE (ratio: ${COMPRESSION_RATIO}x)"
    echo "Ready for upload to R2: $OUTPUT_FILE"
    exit 0
else
    echo "Error: Failed to create archive" >&2
    # Clean up partial files
    if [ -f "$OUTPUT_FILE" ]; then
        rm -f "$OUTPUT_FILE"
    fi
    exit 1
fi
