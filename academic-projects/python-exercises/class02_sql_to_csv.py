# *********************************************************
#
# Description: SQL file parser that converts table data to CSV files.
# Authors: Moshe Yifrah, Chaim Shachar
#
# *********************************************************
import os
import csv
import re

# --- Functions ---

# Load SQL file content
def load_sql_file(file_path):
    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            return file.read()
    except FileNotFoundError as e:
        print(f"File not found: {e}")
        return None
    except IOError as e:
        print(f"Error reading file: {e}")
        return None
    except Exception as e:
        print(f"Unexpected error: {e}")
        return None

# Find table columns from CREATE TABLE statements
def get_table_columns(sql_content, table_name):
    pattern = rf'CREATE TABLE `{table_name}` \((.*?)\);'
    match = re.search(pattern, sql_content, re.DOTALL)

    if match:
        table_schema = match.group(1)
        column_names = re.findall(r'`([^`]+)`', table_schema)
        return column_names
    else:
        return []

# Find rows for a specific table
def find_rows_for_table(sql_content, table_name):
    pattern = rf'INSERT INTO `{table_name}`.*?VALUES \((.*?)\);'
    matches = re.findall(pattern, sql_content, re.DOTALL)
    return matches

# Create CSV file from table data
def create_csv_for_table(table_name, rows, output_dir, sql_content):
    if not rows:
        return None  # No rows to export for this table

    headers = get_table_columns(sql_content, table_name)
    if not headers:
        return None  # No headers found, skip CSV creation

    file_path = os.path.join(output_dir, f"{table_name}.csv")
    try:
        with open(file_path, mode='w', newline='', encoding='utf-8') as csv_file:
            writer = csv.writer(csv_file)
            writer.writerow(headers)

            for row in rows:
                values = re.split(r',\s*(?![^()]*\))', row)
                formatted_values = []

                for value in values:
                    value = value.strip()
                    if value == 'NULL':
                        formatted_values.append(value)
                    else:
                        if not value.startswith("'") and not value.endswith("'"):
                            formatted_values.append(f"'{value}'")
                        else:
                            formatted_values.append(value)

                writer.writerow(formatted_values)

        return file_path
    except IOError as e:
        print(f"Error writing CSV file: {e}")
        return None
    except Exception as e:
        print(f"Unexpected error while creating CSV for table {table_name}: {e}")
        return None

# Find all table names in the SQL file
def find_tables_in_sql(sql_content):
    pattern = r'CREATE TABLE `([a-zA-Z0-9_]+)`'
    tables = re.findall(pattern, sql_content)
    return tables

# Main function to process the SQL file and create CSV files
def process_sql_file(file_path, output_dir):
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    sql_content = load_sql_file(file_path)
    if not sql_content:
        return

    tables = find_tables_in_sql(sql_content)

    for table in tables:
        rows = find_rows_for_table(sql_content, table)

        output_path = create_csv_for_table(table, rows, output_dir, sql_content)
        if output_path:
            print(output_path)  # Print the CSV file path as required

# Define file path and output directory
sql_file_path = 'demo.sql'  # Put the correct path here
output_dir = 'output_csv_files'

# Call the main function
process_sql_file(sql_file_path, output_dir)
