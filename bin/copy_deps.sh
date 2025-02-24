#!/bin/bash
# copy_deps.sh
#
# Usage:
#   ./copy_deps.sh <exe_file_path> [destination_directory]
#
# Example:
#   ./copy_deps.sh test1.exe ./deps
#
# The script uses ldd to list dependencies for the given executable, filters lines that contain "ucrt",
# extracts the DLL paths, and copies them to the destination directory. However, if a DLL is in the Windows
# system path (e.g., /c/WINDOWS/System32/...), it will not be copied.

# Check if at least one parameter is provided
if [ $# -lt 1 ]; then
    echo "Usage: $0 <exe_file_path> [destination_directory]"
    exit 1
fi

EXE_FILE="$1"
DEST_DIR="${2:-./deps}"  # Default destination directory is ./deps if not provided

# Verify that the specified executable exists
if [ ! -f "$EXE_FILE" ]; then
    echo "Error: File '$EXE_FILE' does not exist!"
    exit 1
fi

# Create the destination directory if it doesn't exist
mkdir -p "$DEST_DIR"

echo "Processing file: $EXE_FILE"
echo "Destination directory: $DEST_DIR"

# Use ldd to get the dependencies, filter for lines containing "ucrt", and iterate over each line
ldd "$EXE_FILE" | grep ucrt | while read -r line; do
    # Extract the path after "=>" (expected format: libxxx.dll => /path/to/libxxx.dll (0x...))
    dep_path=$(echo "$line" | awk -F'=> ' '{print $2}' | awk '{print $1}')

    # If the dependency path is empty or not a file, warn and continue to the next line
    if [ -z "$dep_path" ] || [ ! -f "$dep_path" ]; then
        echo "Warning: '$dep_path' not found or invalid."
        continue
    fi

    # Skip copying if the dependency is in a Windows system directory
    case "$dep_path" in
        /c/WINDOWS/* | /c/windows/*)
            echo "Skipping Windows system dependency: $dep_path"
            continue
            ;;
    esac

    echo "Copying $dep_path to $DEST_DIR"
    cp "$dep_path" "$DEST_DIR"
done
