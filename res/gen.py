import os
import argparse


def generate_qresource(directory, prefix, output_file):
    """
    根据给定的目录，生成 Qt .qrc 文件中 <qresource> 节点的内容。
    prefix       : 对应 <qresource prefix="..."> 的值
    output_file  : 要生成的 qrc 文件路径
    """
    # 将目录路径标准化
    directory = os.path.normpath(directory)

    # 准备 XML 头尾
    qresource_header = f'<qresource prefix="/{prefix}">\n'
    qresource_footer = "</qresource>"

    file_entries = []

    for root, dirs, files in os.walk(directory):
        for file_name in files:
            # 得到相对于最初 directory 的相对路径，并将 Windows 下的反斜杠替换成正斜杠
            relative_path = os.path.relpath(
                os.path.join(root, file_name), start=directory
            ).replace("\\", "/")

            # 这里的 alias 可以按你需要来定：
            # 1) 用文件名作为 alias
            # 2) 或者用不带后缀的文件名
            # 3) 甚至干脆不加 alias 属性
            #
            # 以下示例是用文件名(含后缀)作为别名
            alias = file_name

            file_entry = f'    <file alias="{alias}">{relative_path}</file>'
            file_entries.append(file_entry)

    # 组装整个 qresource 内容
    qresource_content = (
        qresource_header + "\n".join(file_entries) + "\n" + qresource_footer
    )

    # 写入到指定文件
    with open(output_file, "w", encoding="utf-8") as f:
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
