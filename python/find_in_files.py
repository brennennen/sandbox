#!/usr/bin/python3
##
# @file
#
# Given a directory, a file pattern, and a string pattern, the script
# finds all instances of the pattern in the specified files and returns
# a list of details on the occurrences found.
#

import os
import sys
import re
import argparse
import logging


class found_token:
"""
A string token and details about it's location in a file.

Attributes:
    value       The string token found.
    file        The file the string token was found in.
    start       The starting index of the token in the file.
    end         The ending index of the token in the file.
    line        The line the token was found started on.
    column      The column the token was found started on.
"""
def __init__(self, value, file, start, end, line, column):
        self.value = value
        self.file = file
        self.start = start
        self.end = end
        self.line = line
        self.column = column


def get_line_and_column(file_string, character_index):
    """
    Gets the line and column of an index in a file string by counting
    the new line characters up to that index.
    @param file_string      String containing the file contents.
    @param character_index  Index to find the line number and column number for.
    @return A tuple containing the line number and column number.
    """
    line_number = 1
    last_new_line_index = 0
    for i, c in enumerate(file_string):
        if i > character_index:
            break
        if c == '\n':
            line_number = line_number + 1
            last_new_line_index = i
    column_number = character_index - last_new_line_index
    return line_number, column_number


def find_in_files(root_directory,
                    file_name_pattern,
                    file_name_ignore_case,
                    token_pattern,
                    token_pattern_ignore_case):
    """
    Searches the specified directory recursively for all files whose name matches
    the specified pattern and contains a string matching a pattern.
    @param root_directory      Directory to search.
    @param file_name_pattern   Pattern of file names to include in search.
    @param token_pattern       String token pattern to find in the file.
    @return A list containing the details of each of the found tokens.
    """
    matched_tokens = []
    file_name_regex = re.compile(file_name_pattern)
    token_regex = re.compile(token_pattern)

    logging.info(f"Finding     Pattern: {token_pattern}    Directory: {root_directory}      Name Pattern: {file_name_pattern}")

    file_list = []
    for dirpath, dirnames, file_names in os.walk(root_directory):
        for file_name in file_names:
            file_list.append(os.path.join(dirpath, file_name))
    file_name_filtered = filter(file_name_regex.search, file_list)

    for matched_file_full_path in file_name_filtered:
        logging.info(f"   Matched file: {matched_file_full_path}")

        with open(matched_file_full_path, "r") as matched_file:
            matched_file_string = matched_file.read()
            test = token_regex.search(matched_file_string)

            matches = token_regex.finditer(matched_file_string)
            for match in matches:
                line_number, column_number = get_line_and_column(matched_file_string, match.start())
                logging.info(f"       Matched Token:   token: {match.group(0)}   start: {match.start()}   end: {match.end()}   line: {line_number}   column: {column_number}")
                matched_token = found_token(match.group(0), matched_file_full_path, match.start(), match.end(), line_number, column_number)
                matched_tokens.append(matched_token)
    logging.info(f"Found Count: {len(matched_tokens)}")
    return matched_tokens


def manage_arguments():
    """
    Manages the arguments to modify the functionality of finding in files. Read the "add_argument"
    sections or run the script with "-h" or "--help" for a list of the arguments.
    """
    script_name = "Find In Files"
    script_usage = "./find_in_files.py --root-directory ./ --file-name-pattern '.*\.c$' --token-pattern 'error' --token-pattern-ignore-case"
    script_description = "Finds the specified patterns in the specified directories."

    argument_parser = argparse.ArgumentParser(prog=script_name, usage=script_usage, description=script_description)
    argument_parser.add_argument( "--root-directory",
                                    default="./",
                                    help="Directory to search. Default: %(default)s")
    argument_parser.add_argument( "--file-name-pattern",
                                    default="\\w*.c",
                                    help="Directory to search. Default: %(default)s")
    argument_parser.add_argument( "--file-name-ignore-case",
                                    default=False,
                                    help="Ignores the case when filtering by file name. Default: %(default)s",
                                    action="store_true")
    argument_parser.add_argument( "--token-pattern",
                                    default="error",
                                    help="File pattern of the files to search for the pattern in. Default: %(default)s")
    argument_parser.add_argument( "--token-pattern-ignore-case",
                                    default=False,
                                    help="Directory to search. Default: %(default)s",
                                    action="store_true")
    argument_parser.add_argument( "--output-file",
                                    default=None,
                                    help="File to print the output to. Default: %(default)s")
    arguments = argument_parser.parse_args()
    logging.debug("Arguments: ")
    logging.debug(f"   root-directory: {arguments.root_directory}")
    logging.debug(f"   file-name-pattern: {arguments.file_name_pattern}")
    logging.debug(f"   file-name-ignore-case: {arguments.file_name_ignore_case}")
    logging.debug(f"   token-pattern: {arguments.token_pattern}")
    logging.debug(f"   token-pattern-ignore-case: {arguments.token_pattern_ignore_case}")
    logging.debug(f"   output-file: {arguments.output_file}")
    return arguments


def main():
    """
    Main entry point into the find in files script. By default the found results are printed to stdout
    and can be written to a file through an argument. To see a complete list of arguments, see the
    manage_arguments function or run the script with "-h" or "--help".
    """
    arguments = manage_arguments()
    results = find_in_files(arguments.root_directory, \
                            arguments.file_name_pattern, \
                            arguments.file_name_ignore_case, \
                            arguments.token_pattern, \
                            arguments.token_pattern_ignore_case)

    print(f"Found {len(results)} results")
    for match in results:
        print(f"token: {match.value}   file:{match.file}   start: {match.start}   end: {match.end}   line: {match.line}   column: {match.column}")


    if __name__ == "__main__":
    logging.basicConfig(format="%(asctime)s | %(name)s | %(levelname)s | %(message)s", stream=sys.stdout, level=logging.DEBUG)
    logging.info("Starting find in files...")
    main()
    logging.info("Ending find in files...")

