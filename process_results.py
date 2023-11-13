import pandas as pd
import numpy as np

def process_results(file_path):
    # Read the data from the CSV file
    data = pd.read_csv(file_path)

    # Group the data by pager and test
    grouped_data = data.groupby(['Pager', 'Test'])

    # Prepare a list to hold the summary data
    summary_data = []

    # Calculate mean and standard deviation for each group
    for name, group in grouped_data:
        mean_time = group['Time'].mean()
        std_time = group['Time'].std()
        mean_memory = group['Memory'].mean()
        std_memory = group['Memory'].std()

        # Append the results to the summary data list
        summary_data.append({
            'Pager': name[0],
            'Test': name[1],
            'Mean Time': mean_time,
            'Std Time': std_time,
            'Mean Memory': mean_memory,
            'Std Memory': std_memory
        })

    # Convert the summary data list to a DataFrame
    summary = pd.DataFrame(summary_data)

    # Print the results
    print(summary)

    # Save the summary to a new CSV file
    summary.to_csv('summary_results.csv', index=False)

if __name__ == "__main__":
    import sys
    if len(sys.argv) != 2:
        print("Usage: python process_results.py <results_file>")
        sys.exit(1)

    process_results(sys.argv[1])
