#!/usr/bin/env python3

import os
import sys
import subprocess
import difflib
import shutil

def main():
    # Set directories
    tests_dir = "tests"
    expected_dir = "expected-outputs"
    
    # Check if directories exist
    if not os.path.isdir(tests_dir):
        print(f"Error: {tests_dir} directory not found")
        sys.exit(1)
    
    if not os.path.isdir(expected_dir):
        print(f"Error: {expected_dir} directory not found")
        sys.exit(1)
    
    # Check if compile executable exists and is executable
    compile_path = "./compile"
    if not os.path.isfile(compile_path) or not os.access(compile_path, os.X_OK):
        print("Error: 'compile' executable not found or not executable")
        sys.exit(1)
    
    # Initialize counters
    total = 0
    passed = 0
    failed = 0
    
    # Print header
    print("===== Test Results =====")
    print("")
    
    # Process each file in the tests directory
    for filename in os.listdir(tests_dir):
        test_file = os.path.join(tests_dir, filename)
        
        # Skip directories
        if not os.path.isfile(test_file):
            continue
        
        expected_file = os.path.join(expected_dir, f"{filename}-out")
        
        # Check if expected output file exists
        if not os.path.isfile(expected_file):
            print(f"Warning: Expected output file not found for {filename}, skipping test")
            continue
        
        print(f"Testing: {filename}")
        
        try:
            # Run the compile command on the test file
            result = subprocess.run([compile_path, test_file], 
                                    capture_output=True, 
                                    text=True, 
                                    check=False)
            
            # Read the expected output
            with open(expected_file, 'r') as f:
                expected_output = f.read()
            
            # Compare outputs using difflib for a more visual diff
            if result.stdout == expected_output:
                print(f"[PASS] {filename}")
                passed += 1
            else:
                print(f"[FAIL] {filename}")
                print("Differences found:")
                
                # Generate and print diff
                diff = difflib.unified_diff(
                    result.stdout.splitlines(),
                    expected_output.splitlines(),
                    fromfile='Output',
                    tofile='Expected',
                    lineterm=''
                )
                
                # Print indented diff
                for line in diff:
                    print(f"  {line}")
                
                failed += 1
            
            total += 1
            print("")
            
        except Exception as e:
            print(f"Error testing {filename}: {str(e)}")
            failed += 1
            total += 1
    
    # Print summary
    print("===== Summary =====")
    print(f"Total tests: {total}")
    print(f"Passed:      {passed}")
    print(f"Failed:      {failed}")
    
    # Exit with appropriate code
    if failed == 0:
        print("All tests passed!")
        sys.exit(0)
    else:
        print("Some tests failed!")
        sys.exit(1)

if __name__ == "__main__":
    main()
