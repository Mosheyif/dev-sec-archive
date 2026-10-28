# *********************************************************
#
# Description: Simple Python functions
# Author: Moshe Yifrah
#
# *********************************************************

def count_char_in_string(string, char):
    """Count the number of occurrences of char in string."""
    return string.count(char)


def reverse_number(num):
    """Reverse any numeric type (int, float), preserving its type and sign."""
    num_type = type(num)

    s = str(num)

    if s.startswith('-'):
        return -num_type(s[1:][::-1])

    return num_type(s[::-1])

def convert_celcius_to_fahrenheit(num):
    """Convert Celcius to Fahrenheit."""
    return num * 1.8 + 32


def is_leap_year(year):
    """Check if year is a leap year."""
    return year % 4 == 0 and year % 100 != 0 or year % 400 == 0


def check_password_complexity(password):
    """Check password complexity and print a score."""
    score = 0
    if len(password) >= 8:
        score += 1
    if any(c.isupper() for c in password):
        score += 1
    if any(c.islower() for c in password):
        score += 1
    if any(c in '@#%&' for c in password):
        score += 1

    print(f"Password complexity: {score * 25}/100")


def cal_sum_of_divisors(num):
    """Calculate the sum of divisors of an integer."""
    if not isinstance(num, int):
        print("Invalid input")
        return 0

    sum_of_divisors = 0
    for x in range(1, num):
        if num % x == 0:
            sum_of_divisors += x
    return sum_of_divisors


def get_min_change(num):
    """Calculate the minimum coins and bills needed for a change."""
    for coin in (200, 100, 50, 20, 10, 5, 2, 1):
        if num == 0:
            break

        count = num // coin
        if count > 0:
            print(f"{coin} x {count}")
            num %= coin


def is_prime(num):
    """Check if num is prime."""
    if num < 2: return False
    if num == 2: return True
    if num % 2 == 0: return False

    for x in range(3, int(num ** 0.5) + 1, 2):
        if num % x == 0:
            return False

    return True


def main():
    """Main menu loop for running exercises interactively."""
    while True:
        print("\n--- Exercise Menu ---")
        print("1. Count character in string")
        print("2. Reverse a number")
        print("3. Convert Celsius to Fahrenheit")
        print("4. Check leap year")
        print("5. Check password complexity")
        print("6. Calculate sum of divisors")
        print("7. Calculate minimum change (Greedy)")
        print("8. Check prime number")
        print("0. Exit")

        choice = input("\nSelect an exercise to run: ")
        print("-" * 21)

        match choice:
            case '1':
                text = input("Enter a string: ")
                char = input("Enter a character to count: ")
                print(f"Result: {count_char_in_string(text, char)}")
            case '2':
                num = input("Enter a number to reverse: ")
                print(f"Result: {reverse_number(num)}")
            case '3':
                celsius = float(input("Enter temperature in Celsius: "))
                print(f"Result: {convert_celcius_to_fahrenheit(celsius)}")
            case '4':
                year = int(input("Enter a year: "))
                print(f"Is leap year? {is_leap_year(year)}")
            case '5':
                pwd = input("Enter a password to check: ")
                check_password_complexity(pwd)
            case '6':
                num = int(input("Enter an integer: "))
                print(f"Sum of divisors: {cal_sum_of_divisors(num)}")
            case '7':
                amount = int(input("Enter amount for change: "))
                get_min_change(amount)
            case '8':
                num = int(input("Enter a number: "))
                print(f"Is prime? {is_prime(num)}")
            case '0':
                print("Exiting program...")
                break
            case _:
                print("Invalid choice, please try again.")


if __name__ == "__main__":
    main()
