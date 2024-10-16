#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>

struct Row {
    std::vector<std::string> columns; // Use a dynamic list of columns
};

// Function to read CSV and return a vector of Rows
std::vector<Row> readCsv(const std::string& filePath) {
    std::vector<Row> rows;
    std::ifstream file(filePath);
    std::string line;

    if (file.is_open()) {
        // Read each line from the CSV file
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string cell;
            Row row;

            // Parse each cell in the row
            while (std::getline(ss, cell, ',')) {
                row.columns.push_back(cell);
            }

            rows.push_back(row);
        }
        file.close();
    }
    return rows;
}

// Function to write a vector of Rows to a CSV
void writeCsv(const std::string& filePath, const std::vector<Row>& rows) {
    std::ofstream file(filePath);
    if (file.is_open()) {
        for (const auto& row : rows) {
            for (size_t i = 0; i < row.columns.size(); ++i) {
                file << row.columns[i];
                if (i < row.columns.size() - 1) file << ",";
            }
            file << "\n";
        }
        file.close();
    }
}

// Function to remove consecutive 'New' rows
std::vector<Row> removeConsecutiveNewRows(const std::vector<Row>& rows) {
    std::vector<Row> cleanedRows;
    for (size_t i = 0; i < rows.size(); ++i) {
        if (i < rows.size() - 1 &&
            rows[i].columns.size() >= 5 && rows[i + 1].columns.size() >= 5 &&
            rows[i].columns[0] == rows[i + 1].columns[0] &&
            rows[i].columns[1] == rows[i + 1].columns[1] &&
            rows[i].columns[2] == rows[i + 1].columns[2] &&
            rows[i].columns[3] == rows[i + 1].columns[3] &&
            rows[i].columns[4] == "New") {
            continue; // Skip adding this row if it matches the condition
        }
        cleanedRows.push_back(rows[i]);
    }
    return cleanedRows;
}

// Function to remove duplicates only if they have "Fill"
std::vector<Row> removeDuplicates(const std::vector<Row>& rows) {
    std::vector<Row> cleanedRows;
    std::unordered_map<std::string, int> seen;
    for (const auto& row : rows) {
        if (row.columns.size() < 5) {
            cleanedRows.push_back(row); // Skip processing if there are not enough columns
            continue;
        }

        std::string key = row.columns[0] + row.columns[1] + row.columns[2] + row.columns[3] + row.columns[4];
        
        if (seen.find(key) == seen.end()) {
            seen[key] = 1;
            cleanedRows.push_back(row);
        } else {
            if (row.columns[4] == "Fill") {
                continue; // Remove duplicate if it has "Fill" status
            } else {
                cleanedRows.push_back(row); // Keep if not "Fill"
            }
        }
    }
    return cleanedRows;
}

int main() {
    std::string inputFile = "C:/Users/HP/Desktop/New Folder/execution_report.csv";
    std::string outputFile = "C:/Users/HP/Desktop/New Folder/modified_execution_report.csv";

    // Step 1: Read the CSV
    std::vector<Row> rows = readCsv(inputFile);

    // Step 2: Remove consecutive 'New' rows
    std::vector<Row> cleanedRows = removeConsecutiveNewRows(rows);

    // Step 3: Remove duplicates only if they have "Fill"
    cleanedRows = removeDuplicates(cleanedRows);

    // Step 4: Write the cleaned data to a new CSV file
    writeCsv(outputFile, cleanedRows);

    std::cout << "Processing complete. Check the file: " << outputFile << std::endl;
    return 0;
}
