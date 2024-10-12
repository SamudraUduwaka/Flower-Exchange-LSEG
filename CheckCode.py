import csv

def compare_csv(file1, file2, ignore_column):
    """
    Compare two CSV files and check if they have the same content, excluding a specified column.
    
    :param file1: Path to the first CSV file
    :param file2: Path to the second CSV file
    :param ignore_column: The column header to ignore during comparison
    :return: True if the CSVs are identical (excluding the ignored column), False otherwise
    """

    with open(file1, 'r') as f1, open(file2, 'r') as f2:
        reader1 = csv.DictReader(f1)
        reader2 = csv.DictReader(f2)

        # Check if both CSVs have the same headers (excluding the ignored column)
        headers1 = [header for header in reader1.fieldnames if header != ignore_column]
        headers2 = [header for header in reader2.fieldnames if header != ignore_column]

        if headers1 != headers2:
            return False

        # Compare each row excluding the ignored column
        for row1, row2 in zip(reader1, reader2):
            # Filter out the ignored column for each row
            filtered_row1 = {key: row1[key] for key in headers1}
            filtered_row2 = {key: row2[key] for key in headers2}

            if filtered_row1 != filtered_row2:
                return False

    return True

# Usage example
file1 = 'C:/Users/HP/Desktop/LSEG Flower/Flower-Exchange-LSEG/Version2/execution_rep.csv'
file2 = 'C:/Users/HP/Desktop/LSEG Flower/Flower-Exchange-LSEG/Version3/execution_report.csv'
ignore_column = 'Transaction Time'  # Column to ignore

if compare_csv(file1, file2, ignore_column):
    print(f"The files '{file1}' and '{file2}' have the same content (ignoring '{ignore_column}' column).")
else:
    print(f"The files '{file1}' and '{file2}' have different content.")