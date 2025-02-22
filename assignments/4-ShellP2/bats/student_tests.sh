#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "cd command with no arguments does nothing" {
    current_dir=$(pwd)
    run ./dsh << EOF
cd
pwd
EOF
   
    expected_output="$current_dir"

    # Check exact match
    [[ "$output" =~ "$expected_output" ]]
    [ "$status" -eq 0 ]

}

@test "cd command with an argument changes directory" {
    current_dir=$(pwd)
    run ./dsh << EOF
cd /temp
pwd
EOF
    
    expected_output="$current_dir"

    # Check exact match
    [[ "$output" =~ "$expected_output" ]]
    [ "$status" -eq 0 ]

}


@test "External command - file not found" {
    run ./dsh << EOF
not_exists
EOF
    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="CommandnotfoundinPATHdsh2>dsh2>cmdloopreturned0"

    # Check exact match
    [[ "$stripped_output" = "$expected_output" ]]
    [ "$status" -eq 0 ]

}

@test "rc returned correctly" {
    rc=$(echo $?)
    run ./dsh << EOF
rc
EOF

    expected_output="$rc"

    # Check exact match
    [[ "$output" =~ "$expected_output" ]]
    [ "$status" -eq 0 ]

}







