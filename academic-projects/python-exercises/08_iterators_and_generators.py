# *********************************************************
#
# Description: Custom iterators, generators, and prime number generator.
# Author: Moshe Yifrah
#
# *********************************************************

# --- Functions ---

class IterReverse:
    def __init__(self, num):
        self.num = num

    def __iter__(self):
        return self

    def __next__(self):
        if self.num < 0:
            raise StopIteration
        current = self.num
        self.num -= 1
        return current

def enumerate_my(sequence):
    for index, value in enumerate(sequence):
        yield index, value

def generate_primes(n):
    def is_prime(num):
        if num <= 1:
            return False
        for i in range(2, int(num**0.5) + 1):
            if num % i == 0:
                return False
        return True

    for num in range(2, n+1):
        if is_prime(num):
            yield num

def pythagorean_triplets(n):
    for x in range(1, n+1):
        for y in range(x, n+1):
            for z in range(y, n+1):
                if x**2 + y**2 == z**2:
                    yield (x, y, z)

# --- Main ---

funct_dict = {
    1: IterReverse,
    2: enumerate_my,
    3: generate_primes,
    4: pythagorean_triplets
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
                num = int(input("Enter the number: "))
                iterator = IterReverse(num)
                for value in iterator:
                    print(value)
            elif choice == 2:
                seq = input("Enter a sequence (comma separated): ").split(",")
                for index, value in enumerate_my(seq):
                    print(f"{index}: {value}")
            elif choice == 3:
                n = int(input("Enter the limit for primes: "))
                for prime in generate_primes(n):
                    print(prime)
            elif choice == 4:
                n = int(input("Enter the limit for Pythagorean triplets: "))
                for triplet in pythagorean_triplets(n):
                    print(triplet)
        else:
            print("Invalid function number.")
    except ValueError:
        print("Insert a valid value.")
