# *********************************************************
#
# Description: Python - OOP basic - classes
# Author: Moshe Yifrah
#
# *********************************************************

# --- Modules ---

from math import sqrt

# --- Classes ---

class Point:
    """This class represents a point (x, y) which must be numbers."""
    def __init__(self, x=0.0, y=0.0):
        if not isinstance(x, (int, float)) or not isinstance(y, (int, float)):
                raise TypeError("x & y must be integers or floats")

        self.x = x
        self.y = y

    def distance_from_origin(self):
        """Return the distance between the received point and its origin."""
        return sqrt(self.x**2 + self.y**2)


class Node:
    """This class represents a node in the list."""
    def __init__(self, value):
        self.value = value
        self.next = None

class List:
    """This class represents a linked list of nodes."""
    def __init__(self):
        self.head = Node(None)
        self.size = 0

    def __len__(self):
        """Return the number of nodes in the list."""
        return self.size

    def get_head(self):
        """Return the first element of the list."""
        return self.head.next.value if self.head.next else None

    def is_empty(self):
        """Return True if the list is empty."""
        return self.size == 0

    def push(self, node):
        """Add a node to the list."""
        cur = self.head
        while cur.next:
            cur = cur.next
        cur.next = node
        self.size += 1

    def pop(self):
        """Remove and return the first node from the list."""
        if self.is_empty():
            return None

        temp = self.head.next
        self.head.next = temp.next
        temp.next = None
        self.size -= 1
        return temp





# --- Main menu ---

def run_all_tests():
    passed = 0
    failed = 0

    def assert_test(test_name, condition, error_msg=""):
        nonlocal passed, failed
        if condition:
            print(f"[V] PASS: {test_name}")
            passed += 1
        else:
            print(f"[X] FAIL: {test_name} - {error_msg}")
            failed += 1

    print("=" * 40)
    print(" Running Automated Edge-Case Tests")
    print("=" * 40)

    print("\n--- Testing Point ---")

    try:
        p1 = Point()
        assert_test("Default initialization (0.0, 0.0)", p1.distance_from_origin() == 0.0)
    except Exception as e:
        assert_test("Default initialization", False, f"Crashed: {e}")

    try:
        p2 = Point(-3, 4)
        assert_test("Distance with negative coordinates (-3, 4)", p2.distance_from_origin() == 5.0)
    except Exception as e:
        assert_test("Distance with negative coordinates", False, f"Crashed: {e}")

    try:
        Point("test", 5)
        assert_test("Reject invalid types", False, "Did not raise exception on string input")
    except TypeError:
        assert_test("Reject invalid types", True)
    except Exception as e:
        assert_test("Reject invalid types", False, f"Raised wrong exception type: {e}")

    print("\n--- Testing Linked List ---")

    try:
        lst = List()
        assert_test("New list is empty (len=0)", lst.is_empty() and len(lst) == 0)
    except Exception as e:
        assert_test("New list is empty", False, f"Crashed: {e}")

    try:
        popped = lst.pop()
        assert_test("Pop from empty list returns None", popped is None)
    except Exception as e:
        assert_test("Pop from empty list", False, f"Crashed on empty pop: {e}")

    try:
        lst.push(Node(10))
        lst.push(Node(20))
        lst.push(Node(None))
        assert_test("Push multiple items (updates len and head)",
                    len(lst) == 3 and lst.get_head() == 10)
    except Exception as e:
        assert_test("Push multiple items", False, f"Crashed: {e}")

    try:
        val1 = lst.pop().value
        val2 = lst.pop().value
        val3 = lst.pop().value
        assert_test("Pop maintains FIFO order (10, 20, None)",
                    val1 == 10 and val2 == 20 and val3 is None)
        assert_test("List is empty after full pop", lst.is_empty())
    except Exception as e:
        assert_test("Pop all items", False, f"Crashed: {e}")

    print("\n" + "=" * 40)
    print(f" Test Summary: {passed} Passed | {failed} Failed")
    print("=" * 40)


def main():
    run_all_tests()


if __name__ == '__main__':
    main()
