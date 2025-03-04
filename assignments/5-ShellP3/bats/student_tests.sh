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

@test "cd to invalid directory returns error" {
    run ./dsh << EOF
cd /this_directory_should_not_exist
rc
EOF
    
    # Should contain non-zero return code
    [[ "$output" =~ [1-9] ]]
    [ "$status" -eq 0 ]
}


@test "rc returned correctly" {
    rc=$(echo $?)
    run ./dsh << EOF
rc
EOF

    expected_output="$rc"

    [[ "$output" =~ "$expected_output" ]]
    [ "$status" -eq 0 ]

}

@test "rc returns zero after successful command" {
    run ./dsh << EOF
echo test
rc
EOF

    # Should contain "0" for successful echo command
    [[ "$output" =~ "0" ]]
    [ "$status" -eq 0 ]
}

@test "rc returns non-zero after failed command" {
    run ./dsh << EOF
ls /non_existent_directory
rc
EOF

    [[ ! "$output" =~ "rc 0" ]]
    [ "$status" -eq 0 ]
}

@test "Dragon command prints dragon" {
    run ./dsh << EOF
dragon
EOF

    # Dragon art should be in output
    [[ "$output" =~ "@%" ]] # checking for characters that should be in the ascii
    [ "$status" -eq 0 ]
}

@test "Exit command exits the shell" {
    run ./dsh << EOF
exit
echo "This should not execute"
EOF

    # Check that shell didn't execute the echo
    [[ ! "$output" =~ "This should not execute" ]]
    [ "$status" -eq 0 ]
}

@test "Simple pipe works correctly" {
    run ./dsh << EOF
echo hello | grep hello
EOF

    
    [[ "$output" =~ "hello" ]]
    [ "$status" -eq 0 ]
}

@test "Multi-stage pipe works correctly" {
    run ./dsh << EOF
echo "pipes test multipipe test" | grep test | wc -w
EOF

    # Should contain "4"
    [[ "$output" =~ "4" ]]
    [ "$status" -eq 0 ]
}

@test "Pipe with command that fails" {
    run ./dsh << EOF
echo "test" | grep "not_in_output"
rc
EOF

    # Should have non-zero rc
    [[ ! "$output" =~ "rc
0" ]]
    [ "$status" -eq 0 ]
}

@test "Exceeding pipe limit shows error" {
    # running 9 piped commands
    run ./dsh << EOF
cmd | cmd | cmd | cmd | cmd | cmd | cmd | cmd | cmd 
EOF

    # Should print message of CMD_ERR_PIPE_LIMIT
    [[ "$output" =~ "error: piping limited to" ]]
    [ "$status" -eq 0 ]
}

@test "Exact pipe limit should not show error" {
    # running 8 piped commands (exact limit)
    run ./dsh << EOF
cmd | cmd | cmd | cmd | cmd | cmd | cmd | cmd
EOF

    # Should NOT print message of CMD_ERR_PIPE_LIMIT
    [[ ! "$output" =~ "error: piping limited to" ]]
    [ "$status" -eq 0 ]
}

@test "Empty command shows warning" {
    run ./dsh << EOF

EOF

    # Should show message for CMD_WARN_NO_CMD
    [[ "$output" =~ "warning: no commands provided" ]]
    [ "$status" -eq 0 ]
}

@test "Handles whitespace in commands" {
    run ./dsh << EOF
   echo    hello    world   
EOF

    # Output should contain the words without extra spaces
    [[ "$output" =~ "hello world" ]]
    [ "$status" -eq 0 ]
}

@test "It handles quoted spaces" {
    run "./dsh" <<EOF                
   echo " hello     world     " 
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '\t\n\r\f\v')

    # Expected output with all whitespace removed for easier matching
    expected_output=" hello     world     dsh3> dsh3> cmd loop returned 0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]
}

@test "Handles backslash escapes" {
    run ./dsh << EOF
echo hello\\world
EOF

    [[ "$output" =~ "hello\\world" ]]
    [ "$status" -eq 0 ]
}

@test "Valid command execution after invalid command" {
    run ./dsh << EOF
notarealcommand
echo this should print
EOF

    # Should show error (CMD_ERR_EXECUTE) for first command but execute second
    [[ "$output" =~ "error: execution failure" ]]
    [[ "$output" =~ "this should print" ]]
    [ "$status" -eq 0 ]
}

@test "Multiple commands execution" {
    run ./dsh << EOF
echo first
echo second
echo third
EOF

    # Output should contain all three strings
    [[ "$output" =~ "first" ]]
    [[ "$output" =~ "second" ]]
    [[ "$output" =~ "third" ]]
    [ "$status" -eq 0 ]
}


@test "Which which ... which?" {
    run "./dsh" <<EOF                
which which
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="/usr/bin/whichdsh3>dsh3>cmdloopreturned0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]
}

# Redirction tests 

@test "Simple output redirection" {
    run ./dsh <<EOF
echo "hello, class" > out.txt
cat out.txt
rm out.txt
EOF

    [[ "$output" =~ "hello, class" ]] 
    [ "$status" -eq 0 ]

}


@test "Append output redirection" {
    run ./dsh <<EOF
echo "hello, class" > out.txt
echo "this is line 2" >> out.txt
cat out.txt
rm out.txt
EOF

    [[ "$output" =~ "hello, class" ]]
    [[ "$output" =~ "this is line 2" ]]
    [ "$status" -eq 0 ]
}

@test "Simple input redirection" {
    #Make input file
    echo "test input file" > input.txt

    run ./dsh <<EOF
cat < input.txt
rm input.txt
EOF

    [[ "$output" =~ "test input file" ]] 
    [ "$status" -eq 0 ]
}

@test "Pipes and redirection" {
    run ./dsh <<EOF
echo "hello world" | grep world > out.txt
cat out.txt
rm out.txt
EOF

    [[ "$output" =~ "hello world" ]] 
    [ "$status" -eq 0 ]

}

@test "Multiple redirections" {
    run ./dsh << EOF
echo "hello class" > out1.txt
echo "this is line 2" > out2.txt
cat out1.txt
cat out2.txt
rm out1.txt out2.txt
EOF

    [[ "$output" =~ "hello class" ]]
    [[ "$output" =~ "this is line 2" ]]
    [ "$status" -eq 0 ]
}

@test "Redirection error handling" {
    run ./dsh <<EOF
cat < non_existent_file.txt
EOF

    #output should contain error message
    [[ "$output" =~ "Error opening input file" ]]
    [ "$status" -eq 0 ]
}

@test "Redirection with built-in command" {
    #Running a command that will set a non-zero exit code
    run ./dsh << EOF
ls /non_existent_directory
rc > rc_output.txt
cat rc_output.txt
rm rc_output.txt
EOF

    #output should contain the non-zero exit code
    [[ "$output" =~ [1-9] ]]
    [ "$status" -eq 0 ]
}