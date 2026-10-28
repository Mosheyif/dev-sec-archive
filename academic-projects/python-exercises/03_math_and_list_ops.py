# *********************************************************
#
# Description: Mathematical operations and list processing functions.
# Author: Moshe Yifrah
#
# *********************************************************
import math

# --- Functions ---

def cal_area():
    r = float(input("Enter the radius: "))

    area = math.pi * r ** 2

    print("The area of the circle is:", area)

def is_squared_list():

    list1 = list(map(int, input("Enter the first list: ").split()))
    list2 = list(map(int, input("Enter the second list: ").split()))

    is_square_result = all(x ** 2 == y for x, y in zip(list1, list2))

    if is_square_result:
        result = [x + y for x, y in zip(list1, list2)]
        print("The series are square-related. The summed series is:", result)
    else:
        print("The second series is not the square result of the first series.")

def positive_only():

    user_input = input("Enter a list: ").split()

    positive_integers = [int(x) for x in user_input if x.isdigit()]

    print(positive_integers)

def divided_list():

    result = [x for x in range(1, 201) if x % 5 == 0 and x % 7 == 0]

    print(result)

def perfect_numbers():
    find_perfects = [
        n for n in range(1, 301)
        if n == sum(m for m in range(1, n) if n % m == 0)]

    print(find_perfects)

def count_numbers():

    numbers_count = {}

    while True:

        number = int(input("Please enter a number: "))

        if number == -1:
            break

        numbers_count[number] = numbers_count.get(number, 0) + 1

    print("The numbers are:")
    for num, count in numbers_count.items():
        times_word = "time" if count == 1 else "times"
        print(f"{num} entered {count} {times_word}")

def run_function(func, number):
    print(f"Function {number} is running")
    func()


# --- Main ---

funct_dict = {
    1: cal_area,
    2: is_squared_list,
    3: positive_only,
    4: divided_list,
    5: perfect_numbers,
    6: count_numbers
}

while True:
    try:
        choice = int(input("Insert the selected function to run (1-6), 0 to break: "))

        if choice == 0:
            print("Exiting the program. Goodbye!")
            break

        elif choice in funct_dict:
            run_function(funct_dict[choice], choice)

    except ValueError:
        print("Insert a valid value")
