#!/usr/bin/env python3

import os
import sys
import subprocess
import difflib
import re

def check_error_output(actual_output, expected_output):
    actual_match = re.search(r'ERROR.*LINE\s+(\d+)', actual_output, re.IGNORECASE)
    expected_match = re.search(r'ERROR.*LINE\s+(\d+)', expected_output, re.IGNORECASE)
    
    if not actual_match:
        return False, "ERROR and LINE not found in actual output"
    
    if not expected_match:
        return False, "ERROR and LINE not found in expected output"
    
    actual_line = actual_match.group(1)
    expected_line = expected_match.group(1)
    
    if actual_line == expected_line:
        return True, f"Error line numbers match: {actual_line}"
    else:
        return False, f"Error line numbers don't match: expected {expected_line}, got {actual_line}"

def normalize_test_output(output, is_test_file):
    if is_test_file:
        return '\n'.join(line for line in output.splitlines() if "exit status: 0" not in line)
    return output

def main():
    if len(sys.argv) < 2 or sys.argv[1] not in ["syntax-tests", "semantic-tests"]:
        print("Usage: ./script.py [syntax-tests | semantic-tests]")
        sys.exit(1)
    
    test_type = sys.argv[1]
    
    base_dir = test_type  # Either "syntax-tests" or "semantic-tests"
    tests_dir = os.path.join(base_dir, "tests")
    expected_dir = os.path.join(base_dir, "expected-outputs")
    
    if not os.path.isdir(base_dir):
        print(f"Error: {base_dir} directory not found")
        sys.exit(1)
    
    if not os.path.isdir(tests_dir):
        print(f"Error: {tests_dir} directory not found")
        sys.exit(1)
    
    if not os.path.isdir(expected_dir):
        print(f"Error: {expected_dir} directory not found")
        sys.exit(1)
    
    compile_path = "./compile"
    if not os.path.isfile(compile_path) or not os.access(compile_path, os.X_OK):
        print("Error: 'compile' executable not found or not executable")
        sys.exit(1)
    
    total = 0
    passed = 0
    failed = 0
    
    print(f"===== {test_type.replace('-', ' ').title()} Test Results =====")
    
    for filename in os.listdir(tests_dir):
        test_file = os.path.join(tests_dir, filename)
        if not os.path.isfile(test_file):
            continue
        
        expected_file = os.path.join(expected_dir, f"{filename}-out")
        if not os.path.isfile(expected_file):
            print(f"Warning: Expected output file not found for {filename}, skipping test")
            continue
        
        try:
            command = f"{compile_path} < {test_file}"
            if test_type == "semantic-tests":
                command = f"{compile_path} --chk_decl < {test_file}"
            
            result = subprocess.run(
                command,
                shell=True,
                capture_output=True,
                text=True,
                check=False
            )
            
            with open(expected_file, 'r') as f:
                expected_output = f.read()
            
            is_error_file = filename.startswith("err")
            is_test_file = filename.startswith("test")
            actual_output = result.stderr if is_error_file else result.stdout
            
            if is_test_file and result.stderr:
                print(f"\n[FAIL] {filename}")
                print("  Expected stderr to be empty, but got:")
                print(f"  {result.stderr}")
                failed += 1
                total += 1
                continue
            
            if is_error_file:
                matches, message = check_error_output(actual_output, expected_output)
                
                if matches:
                    passed += 1
                else:
                    print(f"\n[FAIL] {filename}")
                    print(f"  {message}")
                    print("  Actual output (stderr):")
                    print(f"  {actual_output}")
                    print("  Expected output:")
                    print(f"  {expected_output}")
                    failed += 1
            else:
                normalized_actual = normalize_test_output(actual_output, is_test_file)
                normalized_expected = normalize_test_output(expected_output, is_test_file)
                
                if normalized_actual == normalized_expected:
                    passed += 1
                else:
                    print(f"\n[FAIL] {filename}")
                    print("Differences found:")
                    diff = difflib.unified_diff(
                        normalized_actual.splitlines(),
                        normalized_expected.splitlines(),
                        fromfile='Output',
                        tofile='Expected',
                        lineterm=''
                    )
                    for line in diff:
                        print(f"  {line}")
                    failed += 1
            
            total += 1
            
        except Exception as e:
            print(f"\n[ERROR] {filename}: {str(e)}")
            failed += 1
            total += 1
    
    print("\n===== Summary =====")
    print(f"Total tests: {total}")
    print(f"Passed:      {passed}")
    print(f"Failed:      {failed}")
    
    if failed == 0:
        print("All tests passed!")
        sys.exit(0)
    else:
        print("Some tests failed!")
        sys.exit(1)

if __name__ == "__main__":
    main()

