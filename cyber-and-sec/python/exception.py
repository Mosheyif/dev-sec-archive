# *********************************************************
#
# Description: Python - Exceptions
# Author: Moshe Yifrah
#
# *********************************************************

# --- Classes ---

class PasswordIsTooShort(Exception):
    """This class represents a custom exception raised when password is too short."""
    pass

class PasswordDoesNotContainLowerCaseLetter(Exception):
    """This class represents a custom exception raised when password does not contain lower case letter."""
    pass

class PasswordDoesNotContainUpperCaseLetter(Exception):
    """This class represents a custom exception raised when password does not contain upper case letter."""
    pass

class PasswordDoesNotContainSpecialSymbols(Exception):
    """This class represents a custom exception raised when password does not contain special symbols."""
    pass


# --- Exercise function ---

def validate_password(password):
    """Checks password rules and raises exceptions if it fails."""
    if len(password) < 8:
        raise PasswordIsTooShort()
    if not any(c.isupper() for c in password):
        raise PasswordDoesNotContainUpperCaseLetter()
    if not any(c.islower() for c in password):
        raise PasswordDoesNotContainLowerCaseLetter()
    if not any(c in '@#%&' for c in password):
        raise PasswordDoesNotContainSpecialSymbols()


def check_password_complexity():
    """Handles user input and prints appropriate messages."""
    password = input("Enter your password: ")

    try:
        validate_password(password)

    except PasswordIsTooShort:
        print("Password must be at least 8 characters long")
    except PasswordDoesNotContainUpperCaseLetter:
        print("Password must contain an upper case letter")
    except PasswordDoesNotContainLowerCaseLetter:
        print("Password must contain a lower case letter")
    except PasswordDoesNotContainSpecialSymbols:
        print("Password must contain a special symbol")

    else:
        print("Password is valid")

    finally:
        print("Just for fun")


if __name__ == "__main__":
    check_password_complexity()