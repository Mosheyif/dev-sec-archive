# *********************************************************
#
# Description: Python scopes and modules
# Author: Moshe Yifrah
#
# *********************************************************

# --- Modules ---

import os
import sys
import stat
import areas
import io
from contextlib import redirect_stdout
from pathlib import Path

# --- Exercise functions ---


def is_global(name):
    """Check if a name is a global variable."""
    return name in globals()


def print_areas(radius, height, width):
    """Print the areas of a circle, triangle and rectangle."""
    print(f"Circle area: {areas.get_circle_area(radius)}")
    print(f"Triangle area: {areas.get_triangle_area(height, width)}")
    print(f"Rectangle area: {areas.get_rectangle_area(height, width)}")


def print_os_details():
    """Print some operating system details."""
    print(f"Operating system: {sys.platform}")
    print(f"Logged user: {os.getlogin()}")
    print(f"Current working directory: {os.getcwd()}")


def reverse_cmd_args():
    """Reverse a command line arguments."""
    print(f"Revered command line arguments: {sys.argv[::-1]}")


def def_perms():
    """Asks for a name of file, checks if it has execution permissions.
        if not, gives execute permissions. for owner and group"""
    file_name = input("Enter a file name: ")
    file_path = get_file_path(file_name)
    if file_path is None:
        print("File not found.")
        return
    current_permissions = os.stat(file_path).st_mode
    owner_exec = stat.S_IXUSR
    group_exec = stat.S_IXGRP

    print(f"Current permissions for file: {file_path} are: {stat.filemode(current_permissions)}")

    has_owner_exec = bool(current_permissions & owner_exec)
    has_group_exec = bool(current_permissions & group_exec)

    new_permissions = current_permissions | owner_exec | group_exec

    if not has_owner_exec or not has_group_exec:
        os.chmod(file_path, new_permissions)

    print(f"New permissions for file: {file_path} are: {stat.filemode(new_permissions)}")

def get_file_path(file_name):
    start_dir = Path("/home/moshe/")
    for path in start_dir.rglob(file_name):
        return str(path)
    return None


# --- Tests ---

def run_all_tests() -> bool:
    """Run edge cases on the functions and return success status."""
    print("\n--- Running Tests ---")
    all_passed = True

    # 1. Test: is_global (returns a boolean)
    print("Testing is_global...")
    global_cases = [
        ("os", True, "Existing imported module"),
        ("__name__", True, "Built-in global variable"),
        ("fake_var_123", False, "Non-existent variable"),
    ]
    for arg, expected, desc in global_cases:
        if is_global(arg) != expected:
            print(f"Failed: {desc}. Expected {expected}, got {is_global(arg)}")
            all_passed = False

    # 2. Test: print_areas (captures stdout to check prints)
    print("Testing print_areas...")
    try:
        f = io.StringIO()
        with redirect_stdout(f):
            # Edge case: Zeros
            print_areas(0, 0, 0)
        output = f.getvalue()
        if "Circle area: 0.0" not in output or "Rectangle area: 0" not in output:
            print("Failed: print_areas did not calculate 0 correctly.")
            all_passed = False

        f = io.StringIO()
        with redirect_stdout(f):
            # Edge case: Floats
            print_areas(1.5, 2.5, 3.5)
        if "Triangle area: 4.375" not in f.getvalue():
            print("Failed: print_areas float calculation mismatch.")
            all_passed = False
    except NameError:
        print("Skipped print_areas: 'areas' module not found in this directory.")

    # 3. Test: print_os_details (captures stdout)
    print("Testing print_os_details...")
    f = io.StringIO()
    with redirect_stdout(f):
        print_os_details()
    output = f.getvalue()
    if sys.platform not in output or os.getcwd() not in output:
        print("Failed: print_os_details did not output real system data.")
        all_passed = False

    # 4. Test: reverse_cmd_args (Mocking sys.argv)
    print("Testing reverse_cmd_args...")
    original_argv = sys.argv  # Save the real args to restore later
    try:
        # Edge case 1: Multiple arguments
        sys.argv = ['my_script.py', 'apple', 'banana']
        f = io.StringIO()
        with redirect_stdout(f):
            reverse_cmd_args()
        if "['banana', 'apple', 'my_script.py']" not in f.getvalue():
            print("Failed: reverse_cmd_args multiple args reversal.")
            all_passed = False

        # Edge case 2: No arguments (just the script name)
        sys.argv = ['my_script.py']
        f = io.StringIO()
        with redirect_stdout(f):
            reverse_cmd_args()
        if "['my_script.py']" not in f.getvalue():
            print("Failed: reverse_cmd_args single arg reversal.")
            all_passed = False
    finally:
        sys.argv = original_argv  # Always restore the real arguments!

    if all_passed:
        print("All edge-case tests passed successfully!")
    else:
        print("Some tests failed.")

    return all_passed


# --- Main menu ---

def main():
    """Main menu loop for running modules & OS exercises interactively."""

    exercises = [
        is_global,
        print_areas,
        print_os_details,
        reverse_cmd_args,
        def_perms
    ]

    while True:
        print("\n--- Modules & Scopes Exercise Menu ---")

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
            run_all_tests()
            continue

        try:
            match choice:
                case '1':
                    var_name = input("Enter variable name to check: ")
                    print(f"Result: {is_global(var_name)}")

                case '2':
                    radius = float(input("Enter radius for the circle: "))
                    height = float(input("Enter height for triangle/rectangle: "))
                    width = float(input("Enter width for triangle/rectangle: "))
                    print("\nResults:")
                    print_areas(radius, height, width)

                case '3':
                    print_os_details()

                case '4':
                    reverse_cmd_args()

                case '5':
                    def_perms()

                case _:
                    print("Invalid choice, please try again.")

        except ValueError:
            print("Invalid input! Please enter numbers where required.")
        except Exception as e:
            print(f"An error occurred: {e}")


if __name__ == "__main__":
    main()