# *********************************************************
#
# Description: Implementation of a Base64 encoder and decoder
# with support for URL-safe encoding using bitwise operations.
# Author: Moshe Yifrah
#
# *********************************************************

# --- Modules ---

import sys
import argparse


# --- Classes & Functions ---

class Base64Codec:
    """
    Implementation of Base64 encoder and decoder using low-level bitwise operations.
    Supports both standard and URL-safe alphabets.
    """

    STD_ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
    URL_ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"

    def __init__(self, url_safe: bool = False):
        self.alphabet = self.URL_ALPHABET if url_safe else self.STD_ALPHABET
        self.reverse_alphabet = {char: index for index, char in enumerate(self.alphabet)}

    def encode(self, data: bytes) -> str:
        """
        Encodes bytes data into a Base64 string using bitwise shifting.
        """
        encoded_chars = []

        for i in range(0, len(data), 3):
            chunk = data[i:i + 3]

            # Combine up to 3 bytes into a 24-bit integer
            block = 0
            for j in range(3):
                block <<= 8
                if j < len(chunk):
                    block |= chunk[j]

            # Extract four 6-bit values directly (no manual alignment needed)
            encoded_chars.append(self.alphabet[(block >> 18) & 0x3F])
            encoded_chars.append(self.alphabet[(block >> 12) & 0x3F])

            if len(chunk) > 1:
                encoded_chars.append(self.alphabet[(block >> 6) & 0x3F])
            else:
                encoded_chars.append("=")

            if len(chunk) > 2:
                encoded_chars.append(self.alphabet[block & 0x3F])
            else:
                encoded_chars.append("=")

        return "".join(encoded_chars)

    def decode(self, encoded_str: str) -> bytes:
        """
        Decodes a Base64 string back into bytes using bitwise manipulation.
        """
        encoded_str = encoded_str.rstrip("=")
        decoded_bytes = bytearray()

        for i in range(0, len(encoded_str), 4):
            chunk = encoded_str[i:i + 4]
            block = 0

            # Combine up to four 6-bit values into a 24-bit integer
            for char in chunk:
                block = (block << 6) | self.reverse_alphabet[char]

            # Adjust the block if padding was present in the last chunk
            if len(chunk) == 2:
                block <<= 12
            elif len(chunk) == 3:
                block <<= 6

            # Extract up to three 8-bit values
            decoded_bytes.append((block >> 16) & 0xFF)
            if len(chunk) >= 3:
                decoded_bytes.append((block >> 8) & 0xFF)
            if len(chunk) == 4:
                decoded_bytes.append(block & 0xFF)

        return bytes(decoded_bytes)


# *********************************************************

# --- Main function ---

def main():
    """Main workflow for Base64 command-line encoder/decoder"""

    parser = argparse.ArgumentParser(description="Custom Base64 Encoder & Decoder")

    parser.add_argument("-e", "--encode", type=str, help="String to encode")
    parser.add_argument("-d", "--decode", type=str, help="Base64 string to decode")
    parser.add_argument("--url-safe", action="store_true", help="Use URL-safe alphabet (- and _)")

    args = parser.parse_args()

    codec = Base64Codec(url_safe=args.url_safe)

    if args.encode:
        encoded_result = codec.encode(args.encode.encode('utf-8'))
        print(f"[+] Encoded: {encoded_result}")

    elif args.decode:
        try:
            decoded_result = codec.decode(args.decode).decode('utf-8')
            print(f"[+] Decoded: {decoded_result}")
        except Exception as e:
            print(f"[-] Error during decoding: {e}")
            sys.exit(1)

    else:
        parser.print_help()


if __name__ == "__main__":
    main()