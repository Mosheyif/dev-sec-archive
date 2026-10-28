# *********************************************************
#
# Description: Python files and context managers
# Author: Moshe Yifrah
#
# *********************************************************

# --- Modules ---

from pathlib import Path
from itertools import islice
import json
import csv

# --- Exercise functions ---

def gen_files_of_letters():
    """Generate 26 files named as all English letters and contain their names."""

    letter_names = {
        'a': 'a', 'b': 'bee', 'c': 'cee', 'd': 'dee', 'e': 'e',
        'f': 'ef', 'g': 'gee', 'h': 'aitch', 'i': 'i', 'j': 'jay',
        'k': 'kay', 'l': 'el', 'm': 'em', 'n': 'en', 'o': 'o',
        'p': 'pee', 'q': 'cue', 'r': 'ar', 's': 'ess', 't': 'tee',
        'u': 'u', 'v': 'vee', 'w': 'double-u', 'x': 'ex', 'y': 'wy',
        'z': 'zee'
    }

    script_dir = Path(__file__).parent
    target_dir = script_dir / "ex1files"
    target_dir.mkdir(parents=True, exist_ok=True)

    for letter , name in letter_names.items():
        file_path = target_dir / f"{letter}.txt"
        file_path.write_text(name, encoding='utf-8')


def read_n_lines_from_file(file_path, n):
    """Read the first n lines from a file."""

    with open(file_path, 'r', encoding='utf-8') as file:
        return list(islice(file, n))


def show_covid_stats(file_path):
    """Show statistics from corona.csv file."""
    print("--- COVID-19 Statistics ---")
    print_vaccinated_stats(file_path)
    print_unvaccinated_stats(file_path)
    print_hospitalization_avg(file_path)
    print("---------------------------")


def print_vaccinated_stats(file_path):
    """Calculate and print statistics for vaccinated individuals."""
    conditions = {'Age': lambda x: True, 'Is_vaccinated': lambda x: x == 'Y'}
    data_list = extract_filtered_columns(file_path, conditions)

    if data_list:
        oldest = max(int(person['Age']) for person in data_list)
        youngest = min(int(person['Age']) for person in data_list)
        print(f"The oldest vaccinated person is: {oldest} years old.")
        print(f"The youngest vaccinated person is: {youngest} years old.")
    else:
        print("No vaccinated data found.")


def print_unvaccinated_stats(file_path):
    """Calculate and print statistics for unvaccinated individuals."""
    conditions = {'Age': lambda x: True, 'Is_vaccinated': lambda x: x == 'N'}
    data_list = extract_filtered_columns(file_path, conditions)

    if data_list:
        oldest = max(int(person['Age']) for person in data_list)
        youngest = min(int(person['Age']) for person in data_list)
        print(f"The oldest unvaccinated person is: {oldest} years old.")
        print(f"The youngest unvaccinated person is: {youngest} years old.")
    else:
        print("No unvaccinated data found.")


def print_hospitalization_avg(file_path):
    """Calculate and print the average length of hospitalization."""
    conditions = {'Length_of_hospitalization': lambda x: True}
    data_list = extract_filtered_columns(file_path, conditions)

    if data_list:
        sum_of_hosp = sum(int(person['Length_of_hospitalization']) for person in data_list)
        avg_hosp = sum_of_hosp / len(data_list)
        print(f"The average length of hospitalization is: {avg_hosp:.2f}.")
    else:
        print("No hospitalization data found.")


def gen_custom_file(file_path):
    """Print a menu, get user choices and filters, and generate a custom CSV file."""

    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            header_line = f.readline()
            if not header_line:
                print("The file is empty.")
                return
            headers = header_line.strip().split(',')
    except FileNotFoundError:
        print(f"Error: File '{file_path}' not found.")
        return

    print("\n--- Select Columns for the New File ---")
    for i, header in enumerate(headers, 1):
        print(f"{i}. {header}")

    print("\nEnter the numbers of the columns you want to include, separated by commas (e.g., 1,3,4):")
    choices = input("> ").strip().split(',')

    chosen_columns = []
    for choice in choices:
        try:
            idx = int(choice.strip()) - 1
            if 0 <= idx < len(headers):
                chosen_columns.append(headers[idx])
        except ValueError:
            pass

    if not chosen_columns:
        print("No valid columns selected. Exiting...")
        return

    conditions_dict = {}
    print("\n--- Set Filters ---")
    print("(Press Enter to skip filtering for a specific column)")

    for col in chosen_columns:
        col_lower = col.lower()

        if col_lower == 'gender':
            ans = input(f"Filter {col} (M/F): ").strip().upper()
            if ans in ['M', 'F']:
                conditions_dict[col] = lambda x, a=ans: x.strip().upper() == a
            else:
                conditions_dict[col] = lambda x: True

        elif col_lower == 'age':
            min_a = input(f"Filter {col} - Min Age: ").strip()
            max_a = input(f"Filter {col} - Max Age: ").strip()

            min_val = int(min_a) if min_a.isdigit() else 0
            max_val = int(max_a) if max_a.isdigit() else 120

            if min_a or max_a:
                conditions_dict[col] = lambda x, mn=min_val, mx=max_val: x.isdigit() and (mn <= int(x) <= mx)
            else:
                conditions_dict[col] = lambda x: True

        elif col_lower == 'is_vaccinated':
            ans = input(f"Filter {col} (Y/N): ").strip().upper()
            if ans in ['Y', 'N']:
                conditions_dict[col] = lambda x, a=ans: x.strip().upper() == a
            else:
                conditions_dict[col] = lambda x: True

        else:
            conditions_dict[col] = lambda x: True

    print("\nExtracting data...")
    extracted_data = extract_filtered_columns(file_path, conditions_dict)

    if not extracted_data:
        print("No records matched your filters. File will not be created.")
        return

    out_dir = Path(__file__).parent / "ex3files"
    out_dir.mkdir(parents=True, exist_ok=True)
    new_file_path = out_dir / "custom_output.csv"

    with open(new_file_path, 'w', encoding='utf-8') as out_file:
        out_file.write(','.join(chosen_columns) + '\n')

        for row_dict in extracted_data:
            row_values = [row_dict.get(col, '') for col in chosen_columns]
            out_file.write(','.join(row_values) + '\n')

    print(f"Success! Created '{new_file_path}' with {len(extracted_data)} records.")


def extract_filtered_columns(file_path, conditions_dict):
    """
    Extract columns from a csv file by a dictionary of conditions.
    """
    results = []

    with open(file_path, 'r', encoding='utf-8') as file:
        header_line = file.readline()
        if not header_line:
            return []

        headers = header_line.strip().split(',')

        columns_to_process = []
        for col_name, condition_func in conditions_dict.items():
            if col_name in headers:
                columns_to_process.append((headers.index(col_name), col_name, condition_func))
            else:
                print(f"Warning: Column '{col_name}' not found in headers.")

        for line in file:
            row = line.strip().split(',')

            row_matches = True
            extracted_row = {}

            for idx, col_name, condition_func in columns_to_process:
                if len(row) <= idx:
                    row_matches = False
                    break

                cell_value = row[idx]

                try:
                    if not condition_func(cell_value):
                        row_matches = False
                        break
                except ValueError:
                    row_matches = False
                    break

                extracted_row[col_name] = cell_value

            if row_matches:
                results.append(extracted_row)

    return results


def convert_csv_to_json(csv_file_path, json_file_path):
    """Convert a csv file to a JSON file"""
    with open(csv_file_path, 'r', encoding='utf-8') as csv_file:
        csv_reader = csv.DictReader(csv_file)
        data = list(csv_reader)

    with open(json_file_path, 'w', encoding='utf-8') as json_file:
        json.dump(data, json_file, indent=4)

    print(f"Converted {csv_file_path} to {json_file_path} successfully!")


def merge_all_files_in_directory(directory_path, output_file_path):
    """Merge all files in a directory into a single large file"""
    dir_path = Path(directory_path)
    out_path = Path(output_file_path)

    if not dir_path.is_dir():
        print(f"Error: The directory '{directory_path}' does not exist.")
        return

    with out_path.open('w', encoding='utf-8') as outfile:

        for item_path in dir_path.iterdir():

            if item_path.is_file() and item_path.absolute() != out_path.absolute():

                try:
                    with item_path.open('r', encoding='utf-8') as infile:
                        for line in infile:
                            outfile.write(line)

                        outfile.write('\n')

                    print(f"Merged: {item_path.name}")

                except UnicodeDecodeError:
                    print(f"Skipped (Not a text file): {item_path.name}")
                except Exception as e:
                    print(f"Error reading {item_path.name}: {e}")

    print(f"\nAll files successfully merged into: {out_path.name}")

# --- Main menu ---

def main():
    """Main menu loop for running the excercises."""

    exercises = [
        gen_files_of_letters,
        read_n_lines_from_file,
        show_covid_stats,
        gen_custom_file,
        convert_csv_to_json,
        merge_all_files_in_directory

    ]

    base_dir = Path(__file__).parent / "ex3files"

    while True:
        print("\n--- Files and Context Managers Exercise Menu ---")

        # Dynamically print the menu using the docstrings of the functions
        for i, func in enumerate(exercises, 1):
            print(f"{i}. {func.__doc__}")

        print("-" * 21)
        print("T. Run all edge-case tests")
        print("0. Exit")

        choice = input("\nSelect an exercise to run: ").upper()
        print("-" * 21)

        if choice == '0':
            print("Exiting program...")
            break

        elif choice == 'T':
            #run_all_tests()
            continue

        try:
            match choice:
                case '1':
                    gen_files_of_letters()

                case '2':
                    file_path = input("Enter file path: ")
                    n = int(input("Enter number of lines: "))
                    lines = read_n_lines_from_file(file_path, n)
                    print("".join(lines))

                case '3':
                    show_covid_stats(base_dir / "corona.csv")

                case '4':
                    gen_custom_file(base_dir / "corona.csv")

                case '5':
                    convert_csv_to_json(base_dir / "corona.csv", base_dir / "corona_data.json")

                case '6':
                    dir_path = input("Enter directory path: ")
                    file_path = input("Enter file path: ")
                    merge_all_files_in_directory(dir_path, file_path)


                case _:
                    print("Invalid choice, please try again.")

        except ValueError:
            print("Invalid input! Please enter numbers where required.")
        except Exception as e:
            print(f"An error occurred: {e}")


if __name__ == "__main__":
    main()
