import os
import argparse


def generate_qresource(directory, prefix, output_file):
    # Normalize the directory path
    directory = os.path.normpath(directory)
    # Prepare the XML content
    qresource_header = f'<qresource prefix="/{prefix}">\n'
    qresource_footer = "</qresource>"
    file_entries = []
    alias_counter = 1  # Start the alias numbering

    for root, dirs, files in os.walk(directory):
        for file_name in files:
            # Create a relative path for the file and ensure it uses forward slashes
            relative_path = os.path.relpath(
                os.path.join(root, file_name), start=directory
            ).replace("\\", "/")
            # Construct the file element with an alias
            file_entry = (
                f'    <file alias="{alias_counter}">{prefix}/{relative_path}</file>'
            )
            file_entries.append(file_entry)
            alias_counter += 1  # Increment the alias for the next file

    # Combine all parts of the XML
    qresource_content = (
        qresource_header + "\n".join(file_entries) + "\n" + qresource_footer
    )

    # Write to the output file
    with open(output_file, "w") as f:
        f.write(qresource_content)

    print(f"QResource text has been saved to {output_file}")


def main():
    parser = argparse.ArgumentParser(
        description="Generate a qresource XML block for files in a directory."
    )
    parser.add_argument("directory", type=str, help="The directory to traverse.")
    parser.add_argument(
        "prefix",
        type=str,
        help="The prefix for the qresource, without leading or trailing slashes.",
    )
    parser.add_argument(
        "output_file", type=str, help="The output file to save the qresource text."
    )

    args = parser.parse_args()

    generate_qresource(args.directory, args.prefix, args.output_file)


if __name__ == "__main__":
    main()
