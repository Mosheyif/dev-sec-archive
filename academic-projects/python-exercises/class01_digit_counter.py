# *********************************************************
#
# Description: Digit counting, stack operations, gene filtering, and Caesar cipher.
# Authors: Moshe Yifrah, Chaim Shachar
#
# *********************************************************
# --- Functions ---

def dig_instance():
    num1 = int(input("Enter first number: "))
    num2 = int(input("Enter second number:"))

    print(sum(str(num2).count(digit) for digit in str(num1)))

def stack_operations():
    stack = []
    while True:
        operation = input(
            "Enter operation (i - insert, e - extract, p - print, any other character to end): ").strip().lower()

        if operation == 'i':
            while True:
                item = input("Enter string: ").strip()
                if len(item) > 3:
                    stack.append(item[1:])
                elif item == 'i':
                    continue
                else:
                    operation = item
                    break

        if operation == 'e':
            if stack:
                removed_item = stack.pop()
                print(f"Item removed: {removed_item}")
            else:
                print("The stack is empty, exiting the program.")
                break

        elif operation == 'p':
            if stack:
                print("Stack content:")
                for index, item in enumerate(stack, 1):
                    print(f"{index}. {item}")
            else:
                print("The stack is empty!")

        else:
            print("Unknown operation, ending the program.")
            break

def filter_genes():

    input_data = [
        "B2B,HLA_A,AMHR2,590,0.12",
        "B2B,HLA_A,AMH,591,0.12",
        "B2B,HLA_A,AMICA1,592,0.12",
        "B2B,HLA_A,AMIGO1,593,0.12",
        "B2B,HLA_A,AMIGO2,594,0.12",
        "B2B,HLA_A,AMIGO3,595,0.12",
        "B2B,HLA_A,AMMECR1L,596,0.12",
        "B2B,HLA_A,AMMECR1,597,0.12",
        "B2B,HLA_A,AMN1,598,0.12",
        "B2B,HLA_A,AMN,599,0.12",
        "B2B,HLA_A,AMOTL1,600,0.12",
        "B2B,HLA_A,AMOTL2,601,0.12",
        "B2B,HLA_A,AMOT,602,0.12",
        "B2B,HLA_A,AMPD1,603,0.12",
        "B2B,HLA_A,AMPD2,604,0.12",
        "B2B,HLA_A,AMPD3,605,0.12",
        "B2B,HLA_A,AMPH,606,0.0019",
        "B2B,HLA_A,AMTN,607,0.12",
        "B2B,HLA_A,AMT,608,0.12",
        "B2B,HLA_A,AMY1A,609,0.12",
        "B2B,HLA_A,AMY2A,610,0.0019",
        "B2B,HLA_A,AMY2B,611,0.12",
        "B2B,HLA_A,AMZ1,612,0.12",
        "B2B,HLA_A,AMZ2P1,613,0.12",
        "B2B,HLA_A,AMZ2,614,0.12",
        "B2B,HLA_A,ANAPC10,615,0.12",
        "B2B,HLA_A,ANAPC11,616,0.12",
        "B2B,HLA_A,ANAPC13,617,0.12",
        "B2B,HLA_A,ANAPC16,618,0.12",
        "B2B,HLA_A,ANAPC1,619,0.12",
        "B2B,HLA_A,ANAPC2,620,0.0019",
        "B2B,HLA_A,ANAPC4,621,0.0019",
        "B2B,HLA_A,ANAPC5,622,0.12",
        "B2B,HLA_A,ANAPC7,623,0.12",
        "B2B,HLA_A,ANGEL1,624,0.12",
        "B2B,HLA_A,ANGEL2,625,0.12",
        "B2B,HLA_A,ANGPT1,626,0.12",
        "B2B,HLA_A,ANGPT2,627,0.12",
        "B2B,HLA_A,ANGPT4,628,0.12",
        "B2B,HLA_A,ANGPTL1,629,0.12",
        "B2B,HLA_A,ANGPTL2,630,0.12",
        "B2B,HLA_A,ANGPTL3,631,0.12",
        "B2B,HLA_A,ANGPTL4,632,0.12",
        "B2B,HLA_A,ANGPTL5,633,0.12",
        "B2B,HLA_A,ANGPTL6,634,0.12",
        "B2B,HLA_A,ANGPTL7,635,0.12",
        "B2B,HLA_A,ANG,636,0.12",
        "B2B,HLA_A,ANK1,637,0.0019",
        "B2B,HLA_A,ANK2,638,0.0019",
        "B2B,HLA_A,ANK3,639,0.12",
        "B2B,HLA_A,ANKAR,640,0.12",
        "B2B,HLA_A,ANKDD1A,641,0.12",
        "B2B,HLA_A,ANKFN1,642,0.0019",
        "B2B,HLA_A,ANKFY1,643,0.12",
        "B2B,HLA_A,ANKHD1_EIF4EBP3,644,0.12",
        "B2B,HLA_A,ANKHD1,645,0.12"
    ]

    seen_genes = set()
    result = []

    for line in input_data:
        parts = line.strip().split(',')

        if len(parts) == 5:
            gene = parts[2]
            value = float(parts[4])

            if value < 0.1 and gene not in seen_genes:
                result.append(line)
                seen_genes.add(gene)

    for gene in result:
        print(gene)

def caesar_cipher():

    text = input("Enter the text to encrypt or decrypt: ")
    shift = 13
    result = []

    for char in text:
        if 'a' <= char <= 'z':
            result.append(chr((ord(char) - ord('a') + shift) % 26 + ord('a')))
        elif 'A' <= char <= 'Z':
            result.append(chr((ord(char) - ord('A') + shift) % 26 + ord('A')))
        else:
            result.append(char)

    print("Result:", ''.join(result))

def run_function(func, number):
    print(f"Function {number} is running")
    func()

# --- Main ---

funct_dict = {
    1: dig_instance,
    2: stack_operations,
    3: filter_genes,
    4: caesar_cipher
}

while True:
    try:
        choice = int(input("Insert the selected function to run (1-4) "))

        if choice in funct_dict:
            run_function(funct_dict[choice], choice)
        else:
            break
    except ValueError:
        break
