# *********************************************************
#
# Description: Calculator and number range operations.
# Author: Moshe Yifrah
#
# *********************************************************
import sys


def calculate_difference_and_add():
    num1 = int(input("Insert the first number: "))
    num2 = int(input("Insert the second number: "))
    num3 = int(input("Insert the third number: "))

    largest = max(num1, num2, num3)
    smallest = min(num1, num2, num3)

    difference = largest - smallest

    num1 += difference
    num2 += difference
    num3 += difference

    print("The new values are:")
    print(f"First number: {num1}")
    print(f"Second number: {num2}")
    print(f"Third number: {num3}")

def perform_calculation(action, num1, num2):
    if action == 1:
        return num1 + num2
    elif action == 2:
        return num1 - num2
    elif action == 3:
        return num1 * num2
    elif action == 4:
        if num2 == 0:
            print("Error: Division by zero")
            sys.exit(1)
        return num1 / num2
    elif action == 5:
        if num2 == 0:
            print ("Error: Division by zero")
            sys.exit(1)
        return num1 // num2
    elif action == 6:
        return num1 % num2
    elif action == 7:
        return num1 ** num2
    else:
        print ("Error")
        sys.exit(1)

def calculator():
    num1 = int(input("Enter your first number: "))
    num2 = int(input("Enter your second number: "))
    print("""
    ------------------------------
    C A L C U L A T I O N S
    ------------------------------
    1. Addition
    2. Subtraction
    3. Multiplication
    4. Division
    5. Floor Division
    6. Modulus
    7. Exponent
    ------------------------------
    Enter your choice [1-7] : """)

    action = int(input())
    print(f"The result is: {perform_calculation(action,num1, num2)}")


while True:
    try:
        choice = int(input("Insert the selected function to run (1-2): "))

        if choice == 1:
            print("Function 1 is running")
            calculator()
            break

        elif choice == 2:
            print("Function 2 is running")
            calculate_difference_and_add()
            break

        else:
            print("Please select either 1 or 2")

    except ValueError:
        print("Insert a valid value")
