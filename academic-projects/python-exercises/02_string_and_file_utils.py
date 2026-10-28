# *********************************************************
#
# Description: String manipulation and file type utilities.
# Author: Moshe Yifrah
#
# *********************************************************

# --- Functions ---

def reverse_name():
    name = input("Enter your name: ")
    reverse = ""

    for char in name:
        reverse = char + reverse

    print(reverse)

def file_type():
    file_name = input("Enter file name: ")
    parts = file_name.split('.')

    if len(parts) > 1:
        parts = parts[-1]
    else:
        print ("There is no type")
        return

    print(f"The file type is: {parts}")

def calculate_expression():
    n = int(input("Enter a number: "))
    result = int(str(n)) + int(str(n) * 2) + int(str(n) * 3)
    print(f"{n} + {str(n)*2} + {str(n)*3} = {result}")

def even_list():
    even_numbers = [x for x in range(11) if x % 2 == 0]
    print(even_numbers)

def common_value():
    list1 = input("Enter first list: ").split()
    list2 = input("Enter second list: ").split()
    # Does not insist on numbers
    for n in list1:
        if n in list2:
            print("True")
            return
    print("No values in common were found")

def break_line():
    sentence = input("Enter a sentence: ")
    words = sentence.split()

    for w in words:
        print(w)

# --- Main ---

while True:
    try:
        choice = int(input("Insert the selected function to run (1-6): "))

        if choice == 1:
            print("Function 1 is running")
            reverse_name()
            break

        elif choice == 2:
             print("Function 2 is running")
             file_type()
             break

        elif choice == 3:
            print("Function 3 is running")
            calculate_expression()
            break

        elif choice == 4:
            print("Function 4 is running")
            even_list()
            break

        elif choice == 5:
            print("Function 5 is running")
            common_value()
            break

        elif choice == 6:
            print("Function 6 is running")
            break_line()
            break
        else:
            print("Please select either 1 or 6")

    except ValueError:
        print("Insert a valid value")
