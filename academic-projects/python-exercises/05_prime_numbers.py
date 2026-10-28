# *********************************************************
#
# Description: Prime number processing and supermarket inventory management.
# Author: Moshe Yifrah
#
# *********************************************************

# --- Classes ---

class Product:
    def __init__(self, product_code, category, stock_quantity, min_stock_quantity, price):
        self.product_code = product_code
        self.category = category
        self.stock_quantity = stock_quantity
        self.min_stock_quantity = min_stock_quantity
        self.price = price

class ProductsList:
    def __init__(self):
        self.products = []

    def add_product(self, product):
        self.products.append(product)
        self.products.sort(key=lambda x: x.price)

    def check_and_print_reorder(self):
        for product in self.products:
            if product.stock_quantity < product.min_stock_quantity:
                print(f"Product code: {product.product_code}, Quantity to order: {product.min_stock_quantity - product.stock_quantity}")

    def print_products_by_category(self, category_name):
        print(f"Category: {category_name}")
        found = False
        for product in self.products:
            if product.category == category_name:
                print(f"Product code: {product.product_code}, Price: {product.price}")
                found = True
        if not found:
            print(f"No products found in the category '{category_name}'.")

    def get_price_by_code(self, product_code):
        for product in self.products:
            if product.product_code == product_code:
                return product.price
        return None

    def count_products_below_quantity(self, k):
        return sum(1 for product in self.products if product.stock_quantity < k)
# --- Functions ---

def is_prime(num):
    if num < 2:
        return False
    for i in range(2, int(num ** 0.5) + 1):
        if num % i == 0:
            return False
    return True

def process_primes():
    filename = "primes.txt"
    max_number = 200
    prime_numbers = []

    try:
        with open(filename, "w") as file:
            prime_count = 1
            for number in range(max_number + 1):
                if is_prime(number):
                    file.write(f"{prime_count}. {number}\n")
                    prime_numbers.append(f"{prime_count}. {number}")
                    prime_count += 1
    except IOError:
        print("Error opening or writing to the file.")

    print("Every second prime:")
    for i in range(1, len(prime_numbers), 2):
        print(prime_numbers[i])

def supermarket_inventory():
    supermarket = ProductsList()

    while True:
        print("Select an option:")
        print("1. Add product")
        print("2. Check reorder items")
        print("3. View products by category")
        print("4. Get price by product code")
        print("5. Count products below quantity")

        menu_choice = input("Enter your choice: ")

        if menu_choice == "1":
            try:
                product_code = int(input("Enter product code: "))
                category = input("Enter product category: ")
                stock_quantity = int(input("Enter stock quantity: "))
                min_stock_quantity = int(input("Enter minimum stock quantity: "))
                price = float(input("Enter product price: "))

                if stock_quantity < 0 or min_stock_quantity < 0 or price <= 0:
                    print("Invalid values for stock or price. All values must be positive.")
                    continue

                product = Product(product_code, category, stock_quantity, min_stock_quantity, price)
                supermarket.add_product(product)

            except ValueError:
                print("Invalid input, please try again.")

        elif menu_choice == "2":
            supermarket.check_and_print_reorder()

        elif menu_choice == "3":
            category_name = input("Enter category to view products: ")
            supermarket.print_products_by_category(category_name)

        elif menu_choice == "4":
            try:
                product_code = int(input("Enter product code to get its price: "))
                price = supermarket.get_price_by_code(product_code)
                if price is not None:
                    print(f"The price of the product is: {price}")
                else:
                    print("Product not found.")
            except ValueError:
                print("Invalid product code.")

        elif menu_choice == "5":
            try:
                k = int(input("Enter stock threshold to count products below this quantity: "))
                count = supermarket.count_products_below_quantity(k)
                print(f"Number of products with stock below {k}: {count}")
            except ValueError:
                print("Invalid input for quantity.")

        else:
            print("Invalid choice, please try again.")

def run_function(func, number):
    print(f"Function {number} is running")
    func()
# --- Main ---

funct_dict = {
    1: process_primes,
    2: supermarket_inventory
}

while True:
    try:
        choice = int(input(f"Insert the selected function to run (1-{len(funct_dict)}), 0 to break: "))

        if choice == 0:
            print("Exiting the program. Goodbye!")
            break

        elif choice in funct_dict:
            run_function(funct_dict[choice], choice)

        else:
            print("Invalid function number.")

    except ValueError:
        print("Insert a valid value")
