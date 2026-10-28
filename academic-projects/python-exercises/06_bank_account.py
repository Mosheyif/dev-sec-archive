# *********************************************************
#
# Description: Bank account management with regular and student account types.
# Author: Moshe Yifrah
#
# *********************************************************
import random

# --- Classes ---
class Person:
    def __init__(self, first_name, last_name, address, id_number):
        self.first_name = first_name
        self.last_name = last_name
        self.address = address
        self.id_number = id_number


class BankAccount:
    def __init__(self, account_number, customer, balance):
        self.account_number = account_number
        self.customer = customer
        self.balance = balance

    def deposit(self, amount):
        self.balance += amount

    def withdraw(self, amount):
        if amount > self.balance:
            return False
        self.balance -= amount
        return True

    def __str__(self):
        return (
            f"Account: {self.account_number}\n"
            f"Name: {self.customer.first_name} {self.customer.last_name}\n"
            f"Balance: {self.balance}\n"
        )


class StudentAccount(BankAccount):
    def __init__(self, account_number, customer, balance, institution):
        super().__init__(account_number, customer, balance)
        self.institution = institution

    def __str__(self):
        return (
            f"Student account: {self.account_number}\n"
            f"Name: {self.customer.first_name} {self.customer.last_name}\n"
            f"Balance: {self.balance}\n"
        )

# --- Main ---
HISTORY_FILE = "account_history.txt"

def log_history(action):
    with open(HISTORY_FILE, "a") as file:
        file.write(action + "\n")

def create_account():
    first_name = input("Enter first name: ")
    last_name = input("Enter last name: ")
    address = input("Enter address: ")
    id_number = input("Enter ID number: ")
    balance = float(input("Enter initial balance: "))
    account_type = input("Enter account type (regular/student): ").strip().lower()

    account_number = random.randint(100000, 999999)
    person = Person(first_name, last_name, address, id_number)

    if account_type == "student":
        institution = input("Enter institution name: ")
        account = StudentAccount(account_number, person, balance, institution)
    else:
        account = BankAccount(account_number, person, balance)

    log_history(f"New account, balance: <{balance}>")
    return account

def deposit(account, fee):
    amount = float(input("Enter amount to deposit: "))
    account.deposit(amount - fee)
    log_history(f"Deposit <{amount}>, balance: <{account.balance}>")
    print("Deposit successful.")

def withdraw(account, fee):
    amount = float(input("Enter amount to withdraw: "))
    if account.withdraw(amount + fee):
        log_history(f"Withdraw <{amount}>, balance: <{account.balance}>")
        print("Withdrawal successful.")
    else:
        print("Insufficient funds.")

def print_history():
    try:
        with open(HISTORY_FILE, "r") as file:
            print(file.read())
    except FileNotFoundError:
        print("No history available.")

def main():
    account = create_account()
    fee = 3 if isinstance(account, StudentAccount) else 5

    while True:
        print("\nChoose an action:")
        print("1. Deposit")
        print("2. Withdraw")
        print("3. Print account")
        print("4. Print history")
        print("5. Exit")

        choice = input("Enter your choice: ")

        if choice == "1":
            deposit(account, fee)
        elif choice == "2":
            withdraw(account, fee)
        elif choice == "3":
            print(account)
        elif choice == "4":
            print_history()
        elif choice == "5":
            print("Goodbye!")
            break
        else:
            print("Invalid choice. Try again.")

if __name__ == "__main__":
    main()
