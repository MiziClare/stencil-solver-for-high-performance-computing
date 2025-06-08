#!/bin/bash

# Directory containing the large datafiles 
datafile_source_dir="/mnt/data1/users/forbes/comp328_io"

# Directory to store the symbolic links
linked_datafile_dir="./linked_datafiles"

# Ensure the new directories exist
mkdir -p "$linked_datafile_dir"

# Loop through each file in the source directory
for file in "$datafile_source_dir"/*; do
    # Get the filename from the full path
    filename=$(basename "$file")
    
    # Create a symbolic link in the workspace directory
    ln -s "$file" "$linked_datafile_dir/$filename"

    # Optionally, print what was linked
    echo "Created symlink for: $filename"
done

echo "Workspace setup complete!"