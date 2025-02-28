#!/bin/bash

# Set directories
TESTS_DIR="tests"
EXPECTED_DIR="expected-outputs"

# Check if the directories exist
if [ ! -d "$TESTS_DIR" ]; then
    echo "Error: $TESTS_DIR directory not found"
    exit 1
fi

if [ ! -d "$EXPECTED_DIR" ]; then
    echo "Error: $EXPECTED_DIR directory not found"
    exit 1
fi

# Check if the compile executable exists and is executable
if [ ! -x "./compile" ]; then
    echo "Error: 'compile' executable not found or not executable"
    exit 1
fi

# Initialize counters
total=0
passed=0
failed=0

# Print header
echo "===== Test Results ====="
echo ""

# Process each file in the tests directory
for test_file in "$TESTS_DIR"/*; do
    if [ -f "$test_file" ]; then
        filename=$(basename "$test_file")
        expected_file="$EXPECTED_DIR/${filename}-out"
        
        # Check if expected output file exists
        if [ ! -f "$expected_file" ]; then
            echo "Warning: Expected output file not found for $filename, skipping test"
            continue
        fi
        
        echo "Testing: $filename"
        
        # Create a temporary file for the output
        temp_output=$(mktemp)
        
        # Run the compile command on the test file and save output
        ./compile < "$test_file" > "$temp_output"
        
        # Use diff to compare the outputs
        if diff -u "$temp_output" "$expected_file" > /dev/null; then
            echo "[PASS] $filename"
            passed=$((passed + 1))
        else
            echo "[FAIL] $filename"
            echo "Differences found:"
            # Show the differences with diff
            diff -u "$temp_output" "$expected_file" | sed 's/^/  /'
            failed=$((failed + 1))
        fi
        
        # Remove temporary file
        rm "$temp_output"
        
        total=$((total + 1))
        echo ""
    fi
done

# Print summary
echo "===== Summary ====="
echo "Total tests: $total"
echo "Passed:      $passed"
echo "Failed:      $failed"

# Exit with appropriate code
if [ $failed -eq 0 ]; then
    echo "All tests passed!"
    exit 0
else
    echo "Some tests failed!"
    exit 1
fi
