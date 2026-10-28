# *********************************************************
#
# Description: URL domain parsing and regular expression utilities.
# Author: Moshe Yifrah
#
# *********************************************************

import re

# --- Functions ---

def print_website_type(file_name):
    with open(file_name, 'r') as file:
        for line in file:
            line = line.strip()
            if line:
                parts = line.split('.')
                if len(parts) >= 3:
                    domain_type = parts[-2]
                    print(f"{domain_type}")

def find_dates(text):
    # Regular expression to match various date formats
    date_pattern = r'\b(\d{1,2})([-/.\s])(\d{1,2})\2(\d{2,4})\b'

    # Find all matches based on the pattern
    dates = re.findall(date_pattern, text)

    # Format the dates properly by joining the components
    formatted_dates = [f"{date[0]}{date[1]}{date[2]}{date[1]}{date[3]}" for date in dates]

    return formatted_dates

def replace_double_lowercase(input_string):
    # Regular expression to find two consecutive identical lowercase letters
    return re.sub(r'([a-z])\1', lambda match: match.group(0).upper(), input_string)

# --- Main ---

funct_dict = {
    1: print_website_type,
    2: find_dates,
    3: replace_double_lowercase
}

while True:
    try:
        print("Insert the selected function to run, 0 to break")
        for key, func in funct_dict.items():
            print(f"{key}: {func.__name__}")

        choice = int(input())

        if choice == 0:
            print("Exiting the program. Goodbye!")
            break

        elif choice in funct_dict:
            print(f"Function {choice} is running")
            if choice == 1:
                print_website_type("websites.txt")
            elif choice == 2:
                text = ("There is a large variety of formats for dates in use, which differ in "
                        "the order of date components (e.g. 31/05/1999, 05/31/1999), component"
                        "separators (e.g. 31.05.1999, 31/05/1999, 31-05-1999), whether leading "
                        "zeros are included (e.g. 31/5/1999 vs. 31/05/1999) and whether all four "
                        "digits of the year are written (e.g., 31.05.1999 vs. 31.05.99).")
                dates = find_dates(text)
                for date in dates:
                    print(date)
            elif choice == 3:
                user_input = input("Enter a string: ")
                result = replace_double_lowercase(user_input)
                print("Modified string:", result)
        else:
            print("Invalid function number.")
    except ValueError:
        print("Insert a valid value.")
