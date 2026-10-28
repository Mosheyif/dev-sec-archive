# *********************************************************
#
# Description: Python bit complicated functions program
# Author: Moshe Yifrah
#
# *********************************************************

# --- Modules ---

import ast
from functools import reduce

# --- Exercise functions ---

def rm_words_from_list(lst, words_list):
    """Remove words from a list."""
    lst[:] = filter(lambda word: word not in words_list, lst)
    return lst


def sort_list_numeric_strings(lst):
    """Sort a list of numeric strings."""
    lst.sort(key=lambda x: int(x))
    return lst


def sum_list(lst):
    """Sum a list of numbers."""
    if not lst or any(not isinstance(x, int) for x in lst):
        return 0
    return reduce(lambda x, y: x + y, lst)


def get_evens_list(lst):
    """Get a list of even numbers from a list."""
    if any(not isinstance(x, int) for x in lst):
        return []
    return [x for x in lst if x % 2 == 0]


def make_ten_percent_discount(dct):
    """Discount 10% of each product's price in dictionary."""
    return {product: price * 0.9 for product, price in dct.items()}


def get_gematria_value(phrase):
    """Calculate gematria value from phrase in Hebrew."""
    gematria_dict = {
        'א': 1, 'ב': 2, 'ג': 3, 'ד': 4, 'ה': 5, 'ו': 6, 'ז': 7, 'ח': 8,
        'ט': 9, 'י': 10, 'כ': 20, 'ל': 30, 'מ': 40, 'נ': 50, 'ס': 60,
        'ע': 70, 'פ': 80, 'צ': 90, 'ק': 100, 'ר': 200, 'ש': 300,
        'ת': 400, 'ך': 500, 'ם': 600, 'ן': 700, 'ף': 800, 'ץ': 900
    }

    if not isinstance(phrase, str):
        return 0

    return sum(gematria_dict.get(c, 0) for c in phrase)


def check_credit_card_number(number):
    """Check if credit card number is valid using the Luhn algorithm."""
    reversed_digits = [int(d) for d in str(number)[::-1]]

    total = sum(
        d if i % 2 == 0 else (d * 2 - 9 if d * 2 > 9 else d * 2)
        for i, d in enumerate(reversed_digits)
    )

    return total % 10 == 0


# --- Tests ---

def run_all_tests() -> bool:
    """Run edge cases on the functions and return success status."""
    print("\n--- Running Tests ---")
    all_passed = True

    test_cases = [
        # rm_words_from_list
        (rm_words_from_list, (["hi", "hello", "hi"], ["hi"]), ["hello"], "rm_words standard"),
        (rm_words_from_list, ([], ["hi"]), [], "rm_words empty list"),
        (rm_words_from_list, (["apple"], ["banana"]), ["apple"], "rm_words no match"),

        # sort_list_numeric_strings
        (sort_list_numeric_strings, (["12", "10", "8", "45"],), ["8", "10", "12", "45"], "sort_numeric standard"),
        (sort_list_numeric_strings, (["-5", "0", "10"],), ["-5", "0", "10"], "sort_numeric negative"),
        (sort_list_numeric_strings, ([],), [], "sort_numeric empty list"),

        # sum_list
        (sum_list, ([1, 2, 3],), 6, "sum_list valid integers"),
        (sum_list, ([],), 0, "sum_list empty list"),
        (sum_list, ([4, 'e', 3],), 0, "sum_list invalid types"),

        # get_evens_list
        (get_evens_list, ([1, 2, 3, 4],), [2, 4], "get_evens standard"),
        (get_evens_list, ([1, 3, 5],), [], "get_evens no evens"),
        (get_evens_list, ([4, 'e', 3],), [], "get_evens invalid types"),

        # make_ten_percent_discount
        (make_ten_percent_discount, ({"Product": 10, "Item2": 20},), {"Product": 9.0, "Item2": 18.0},
         "discount standard"),
        (make_ten_percent_discount, ({},), {}, "discount empty dict"),

        # get_gematria_value
        (get_gematria_value, ("שלום",), 936, "gematria standard"),
        (get_gematria_value, ("Hello",), 0, "gematria non-hebrew"),
        (get_gematria_value, ("",), 0, "gematria empty string"),
        (get_gematria_value, (123,), 0, "gematria invalid type"),

        # check_credit_card_number
        (check_credit_card_number, ("5558880600750848",), True, "credit_card valid (16 chars)"),
        (check_credit_card_number, ("5558880600750849",), False, "credit_card invalid"),
        (check_credit_card_number, ("378282246310005",), True, "credit_card valid (Amex 15 chars)")
    ]

    for func, args, expected, test_name in test_cases:
        try:
            result = func(*args)
            if result != expected:
                print(f"Failed: {test_name}. Expected {expected}, got {result}")
                all_passed = False
        except Exception as e:
            print(f"Error in {test_name}: {e}")
            all_passed = False

    if all_passed:
        print("All edge-case tests passed successfully!")
    else:
        print("Some tests failed.")

    return all_passed


# --- Main menu ---

def main():
    """Main menu loop for running exercises interactively."""

    exercises = [
        rm_words_from_list,
        sort_list_numeric_strings,
        sum_list,
        get_evens_list,
        make_ten_percent_discount,
        get_gematria_value,
        check_credit_card_number
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
            run_all_tests()
            continue

        try:
            match choice:
                case '1':
                    lst = ast.literal_eval(input("Enter a list (e.g. ['hi', 'hello']): "))
                    words = ast.literal_eval(input("Enter words to remove (e.g. ['hi']): "))
                    print(f"Result: {rm_words_from_list(lst, words)}")

                case '2':
                    lst = ast.literal_eval(input("Enter a list of numeric strings (e.g. ['12', '10']): "))
                    print(f"Result: {sort_list_numeric_strings(lst)}")

                case '3':
                    lst = ast.literal_eval(input("Enter a list of numbers (e.g. [1, 2, 3]): "))
                    print(f"Result: {sum_list(lst)}")

                case '4':
                    lst = ast.literal_eval(input("Enter a list of numbers (e.g. [1, 2, 3, 4]): "))
                    print(f"Result: {get_evens_list(lst)}")

                case '5':
                    dct = ast.literal_eval(input("Enter a dictionary (e.g. {'Product': 10}): "))
                    print(f"Result: {make_ten_percent_discount(dct)}")

                case '6':
                    phrase = input("Enter a Hebrew phrase: ")
                    print(f"Result: {get_gematria_value(phrase)}")

                case '7':
                    card = input("Enter a credit card number: ")
                    print(f"Result: {check_credit_card_number(card)}")

                case _:
                    print("Invalid choice, please try again.")

        except (ValueError, SyntaxError):
            print(
                "Invalid input format! Please use valid Python syntax (e.g., brackets for lists, quotes for strings).")


if __name__ == "__main__":
    main()