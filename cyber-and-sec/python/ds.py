# *********************************************************
#
# Description: Simple Python functions using data structures
# Author: Moshe Yifrah
#
# *********************************************************

# --- Modules ---

from collections import Counter

# --- Exercise functions ---

def rm_none_str_from_list(lst):
    """Remove none string from list."""
    for i in reversed(range(len(lst))):
        if type(lst[i]) != str:
            lst.pop(i)
    return lst


def get_dict_from_str(my_str):
    """Convert string to dictionary of each letter and its occurrences."""
    return dict(Counter(my_str))


def get_common_elements(lst1, lst2):
    """Get common elements from two lists."""
    return list(set(lst1) & set(lst2))


def get_unique_values_from_dict(my_dict):
    """Get a list of unique values from a dictionary."""
    return list(set(my_dict.values()))


def left_rotate(lst, n):
    """Left rotate a list by n."""
    if not lst: return []
    n = n % len(lst)
    return lst[n:] + lst[:n]


def rm_2nd_num_in_loop(lst):
    """Remove and print every second number from the list until its empty."""
    idx = 1
    while lst:
        idx = idx % len(lst)
        print(f"{lst.pop(idx)}")
        idx += 1


def convert_dict_to_list_of_tuples(my_dict):
    """Convert dictionary to a list of tuples."""
    return list(my_dict.items())


def get_keys_of_min_max_values_from_dict(my_dict):
    """Get keys of minimum and maximum values from dictionary."""
    if not my_dict:
        return
    print(f"Max: {max(my_dict, key=my_dict.get)}")
    print(f"Min: {min(my_dict, key=my_dict.get)}")


def rm_str2_chars_from_str1(str1, str2):
    """Remove from first string the characters in second string."""
    translation_table = str1.maketrans("", "", str2)
    return str1.translate(translation_table)


# --- Tests ---

def run_all_tests() -> bool:
    """Run simple edge cases on the functions and return success status."""
    print("\n--- Running Tests ---")
    all_passed = True

    if rm_none_str_from_list([1, "a", None, "b", 3.14]) != ["a", "b"]:
        print("rm_none_str_from_list failed")
        all_passed = False

    if get_dict_from_str("hello") != {'h': 1, 'e': 1, 'l': 2, 'o': 1}:
        print("get_dict_from_str failed")
        all_passed = False

    if set(get_common_elements([1, 2, 3], [])) != set():
        print("get_common_elements failed")
        all_passed = False

    if left_rotate([1, 2, 3], 5) != [3, 1, 2]:
        print("left_rotate failed")
        all_passed = False

    if rm_str2_chars_from_str1("hello world", "lo") != "he wrd":
        print("rm_str2_chars_from_str1 failed")
        all_passed = False

    if all_passed:
        print("All tests passed successfully!")
    else:
        print("Some tests failed.")

    return all_passed


# --- Main menu ---

def main():
    """Main menu loop for running exercises interactively."""

    exercises = [
        rm_none_str_from_list,
        get_dict_from_str,
        get_common_elements,
        get_unique_values_from_dict,
        left_rotate,
        rm_2nd_num_in_loop,
        convert_dict_to_list_of_tuples,
        get_keys_of_min_max_values_from_dict,
        rm_str2_chars_from_str1
    ]

    while True:
        print("\n--- Exercise Menu ---")

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
            test_status = run_all_tests()
            continue

        match choice:
            case '1':
                print(f"Result: {rm_none_str_from_list([1, 2, 'fr', '214r3', 'f'])}")
            case '2':
                my_str = input("Enter a string: ")
                print(f"Result: {get_dict_from_str(my_str)}")
            case '3':
                lst1 = [1, 2, 3, 4, 5]
                lst2 = [1, 3, 2, 's', 0]
                print(f"Result: {get_common_elements(lst1, lst2)}")
            case '4':
                my_dict = {'a': 1, 'b': 2, 'c': 2, 'd': 1, 'e': 5}
                print(f"Result: {get_unique_values_from_dict(my_dict)}")
            case '5':
                lst = [1, 2, 3, 4, 5]
                n = int(input("Enter a number (n): "))
                print(f"Result: {left_rotate(lst, n)}")
            case '6':
                lst = [1, 2, 3, 4, 5]
                rm_2nd_num_in_loop(lst)
            case '7':
                my_dict = {'a': 1, 'b': 2, 'c': 2, 'd': 1, 'e': 5}
                print(f"Result: {convert_dict_to_list_of_tuples(my_dict)}")
            case '8':
                my_dict = {'a': 1, 'b': 2, 'c': 2, 'd': 1, 'e': 5}
                get_keys_of_min_max_values_from_dict(my_dict)
            case '9':
                str1 = input("Enter first string: ")
                str2 = input("Enter second string: ")
                print(f"Result: {rm_str2_chars_from_str1(str1, str2)}")
            case _:
                print("Invalid choice, please try again.")


if __name__ == "__main__":
    main()
