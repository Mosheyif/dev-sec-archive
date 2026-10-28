# *********************************************************
#
# Description: Recursive array symmetry check and exponential series calculation.
# Author: Moshe Yifrah
#
# *********************************************************
import math

# --- Functions ---

def convert_to_float(array):
    try:
        return [float(x.strip()) for x in array]
    except ValueError:
        return None

def is_symmetric_sum_array():
    try:
        # Getting input from the user and splitting it by spaces
        user_input = input("Enter the array (space-separated values): ").split()

        # Trying to convert the input into a list of floats
        float_array = convert_to_float(user_input)

        # If conversion fails, raise an exception to handle invalid input
        if float_array is None:
            raise ValueError("Please enter valid numeric values.")

        # Check if the array length is even
        if len(float_array) % 2 != 0:
            raise ValueError("The array length must be even.")

        # Recursive function to check if sums of opposite elements are equal
        def check_recursive(arr, left, right, expected_sum):
            if left >= right:
                return True
            if arr[left] + arr[right] != expected_sum:
                return False
            return check_recursive(arr, left + 1, right - 1, expected_sum)

        # Calculate the sum of the first and last elements
        first_pair_sum = float_array[0] + float_array[-1]

        # Check if the array is a symmetric sum array
        if check_recursive(float_array, 0, len(float_array) - 1, first_pair_sum):
            print("The array is a symmetric sum array.")
        else:
            print("The array is not a symmetric sum array.")

    except ValueError as e:
        # Handle any ValueErrors (invalid input or invalid array length)
        print(f"Error: {e}")


def calculate_exponential():
    try:
        # Get inputs for x and precision D
        x_value = float(input("Enter the value of x (decimal): "))
        precision_value = float(input("Enter the precision level D (decimal): "))

        if x_value <= 0 or precision_value <= 0:
            raise ValueError("Both x and precision D must be positive.")

        # Recursive function to calculate the series
        def calculate_recursive(x, n, precision, current_sum=0, term_count=0):

            next_term = (x ** n) / math.factorial(n)  # Calculate the next term in the series

            # If the next term is smaller than precision, stop recursion
            if next_term < precision:
                return current_sum, term_count  # Don't add it if it's too small

            return calculate_recursive(x, n + 1, precision, current_sum + next_term, term_count + 1)

        # Calculate the result using the recursive function
        result, num_terms = calculate_recursive(x_value, 0, precision_value)

        # Print the result
        print(f"e^{x_value} ≈ {result}")
        print(f"Number of terms used: {num_terms}")

    except ValueError as e:
        print(f"Error: {e}")  # Handle invalid input errors like non-numeric input or incorrect precision


# --- Main ---

funct_dict = {
    1: is_symmetric_sum_array,
    2: calculate_exponential
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
            funct_dict[choice]()

        else:
            print("Invalid function number.")

    except ValueError:
        print("Insert a valid value")

