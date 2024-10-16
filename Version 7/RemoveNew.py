import pandas as pd

# Load the CSV file
df = pd.read_csv('C:/Users/HP/Desktop/LSEG Flower/Flower-Exchange-LSEG/Version 6/execution_report.csv')

# Identify rows to drop for the first condition (consecutive rows with "New")
rows_to_drop = []

# Loop through the DataFrame for the first condition
for i in range(len(df) - 1):
    if (df.iloc[i, :4].equals(df.iloc[i + 1, :4])) and (df.iloc[i, 4] == 'New'):
        rows_to_drop.append(i)

# Drop the identified rows from the first condition
df.drop(index=rows_to_drop, inplace=True)

# Reset the index after the first drop
df.reset_index(drop=True, inplace=True)

# Identify rows to drop for the second condition (duplicates)
rows_to_drop = []
seen = {}

# Loop through the DataFrame for the second condition
for i in range(len(df)):
    key = tuple(df.iloc[i, :5])  # Create a tuple of the first five columns as a key
    if key in seen:
        # If the fifth column is not "PFill", mark the current index for dropping
        if df.iloc[i, 4] != 'PFill':
            rows_to_drop.append(i)
    else:
        # Mark the key as seen
        seen[key] = i

# Drop the identified rows from the second condition
df.drop(index=rows_to_drop, inplace=True)

# Reset the index after the second drop
df.reset_index(drop=True, inplace=True)

# Save the modified DataFrame back to CSV
df.to_csv('C:/Users/HP/Desktop/LSEG Flower/Flower-Exchange-LSEG/Version 6/modified_execution_report.csv', index=False)

print("Rows with 'New' entity removed for consecutive matches, and duplicates removed where the fifth column is not 'PFill'.")
