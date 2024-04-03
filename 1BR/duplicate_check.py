def extract_last_8_chars(s):
    """
    Extract the last 8 characters of a string.
    """
    return s[-8:]

def find_duplicates(file_path):
    """
    Find duplicate strings among the last 8 characters of each line in a file.
    """
    last_8_chars_set = set()
    duplicates = set()

    with open(file_path, 'r') as file:
        for line in file:
            # Extract the last 8 characters of the string
            last_8_chars = extract_last_8_chars(line.strip())

            # Check if last 8 characters are a duplicate
            if last_8_chars in last_8_chars_set:
                duplicates.add(last_8_chars)
            else:
                last_8_chars_set.add(last_8_chars)

    return duplicates

def main():
    file_path = input("Enter the path to the file: ")
    duplicates = find_duplicates(file_path)

    if duplicates:
        print("Duplicates found:")
        for duplicate in duplicates:
            print(duplicate)
    else:
        print("No duplicates found.")

if __name__ == "__main__":
    main()
