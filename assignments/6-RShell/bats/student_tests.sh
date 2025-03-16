#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}


# Local Shell Tests

@test "Local: Simple command execution" {
    run bash -c "echo 'echo hello world' | ./dsh"
    
    # Verify output
    [[ "$output" == *"hello world"* ]]
}

@test "Local: Command with arguments" {
    run bash -c "echo 'ls -la' | ./dsh"
    
    # Check if command executed correctly
    [[ "$status" -eq 0 ]]
    [[ "$output" == *"total"* ]]  # 'ls -la' output starts with "total"
}

@test "Local: Pipe commands" {
    run bash -c "echo 'echo hello world | grep hello' | ./dsh"
    
    # Verify output
    [[ "$output" == *"hello world"* ]]
}

@test "Local: Multiple piped commands" {
    run bash -c "echo 'echo -e \"line1\\nline2\\nline3\" | wc -l' | ./dsh"
    
    echo "Output: $output"
    
    # Should show 3 lines
    [[ "$output" == *"3"* ]]
}

@test "Local: Input redirection" {

    # Create temp directory and input file
    TEST_DIR=$(mktemp -d)
    echo "test_input_data" > $TEST_DIR/test_input.txt

    run bash -c "echo 'cat < $TEST_DIR/test_input.txt' | ./dsh"
    
    # Check output matches input data
    [[ "$output" == *"test_input_data"* ]]

    rm -rf $TEST_DIR
}

@test "Local: Output redirection" {
    # Create temp directory for test
    TEST_DIR=$(mktemp -d)

    run bash -c "echo 'echo hello world > $TEST_DIR/test_output.txt' | ./dsh"
    
    # Verify file was created with correct content
    [[ -f "$TEST_DIR/test_output.txt" ]]
    [[ "$(cat $TEST_DIR/test_output.txt)" == "hello world" ]]

    rm -rf $TEST_DIR
}

@test "Local: cd command" {
    current_dir=$(pwd)
    # Run the command
    run bash -c 'cat <<EOF | ./dsh
pwd
cd ..
pwd
exit
EOF'
    
    # Verify the directory
    [[ "$output" =~ "$current_dir" ]]
    [ "$status" -eq 0 ]
}

@test "Local: cd command with invalid directory" {
    current_dir=$(pwd)
    # Run the command
    run bash -c 'cat <<EOF | ./dsh
pwd
cd /this_directory_should_not_exist
rc
exit
EOF'
    
    # Should contain non-zero return code
    [[ "$output" =~ [1-9] ]]
    [ "$status" -eq 0 ]
}

@test "Local: rc for successfulcommand" {
    rc=$(echo $?)
    # Run the command
    run bash -c 'cat <<EOF | ./dsh
echo test
rc
exit
EOF'
    
    # Verify code is 0
    [[ "$output" =~ "0" ]]
    [ "$status" -eq 0 ]
}

@test "Local: rc for unsuccessful command" {
    rc=$(echo $?)
    # Run the command
    run bash -c 'cat <<EOF | ./dsh
rc
exit
EOF'
    
    # Verify code is non-zero
    [[ ! "$output" =~ "rc 0" ]]
    [ "$status" -eq 0 ]
}

@test "Local: dragon command" {
    rc=$(echo $?)
    # Run the command
    run bash -c 'cat <<EOF | ./dsh
dragon
exit
EOF'
    
    # Verify code is non-zero
    [[ "$output" =~ "@%" ]] # checking for characters that should be in the ascii
    [ "$status" -eq 0 ]
}

@test "Local: Exit command" {
    # Start dsh in background and send exit command
    run bash -c "echo 'exit' | ./dsh"
    
    # Check if exit was successful
    [[ "$status" -eq 0 ]]
}

@test "Local: Non-existent command" {
    run bash -c "echo 'nonexistentcommand' | ./dsh"
    
    # Should show error message
    [[ "$output" == *"not found"* || "$output" == *"No such file"* ]]
    [[ "$status" -eq 0 ]]
}


@test "Server: check server starts and exits" {
    # Start server in background
    ./dsh -s -i 0.0.0.0 -p 7890 &
    server_pid=$!
    
    # Give the server time to start
    sleep 1
    
    # Use client to connect and send stop-server command
    echo "stop-server" | ./dsh -c -i 0.0.0.0 -p 7890
    
    # Wait for server to exit
    wait $server_pid
    
    # Check exit status
    [ "$?" -eq 0 ]
}

@test "Server: check client connection" {
    # Start server
    ./dsh -s -i 0.0.0.0 -p 7890 &
    server_pid=$!
    
    sleep 1
    
    # First client connection
    run bash -c "echo 'echo client1' | ./dsh -c -i 10.246.251.11 -p 7890"
    [[ "$output" == *"client1"* ]]
    
    # Send stop-server command
    echo "stop-server" | ./dsh -c -i 10.246.251.11 -p 7890
    
    wait $server_pid
}

@test "Client-Server: Simple command execution" {
    # Start server
    ./dsh -s -i 0.0.0.0 -p 7890 &
    server_pid=$!
    
    sleep 1
    
    # Run a simple command and capture output
    run bash -c "echo 'echo hello world' | ./dsh -c -i 10.246.251.11 -p 7890"
    
    # Send stop-server command
    echo "stop-server" | ./dsh -c -i 10.246.251.11 -p 7890
    
    wait $server_pid
    
    # Check output contains expected string
    [[ "$output" == *"hello world"* ]]
}

@test "Client-Server: Multi-command execution" {
    # Start server
    ./dsh -s -i 0.0.0.0 -p 7890 &
    server_pid=$!
    
    sleep 1
    
    # Run multiple commands
    run bash -c "echo -e 'pwd\nls -la\necho hello world' | ./dsh -c -i 10.246.251.11 -p 7890"
    
    # Send stop-server command
    echo "stop-server" | ./dsh -c -i 10.246.251.11 -p 7890
    
    wait $server_pid
    
    # Check for expected outputs
    [[ "$output" == *"hello world"* ]]
    [[ "$output" == *".c"* ]] || [[ "$output" == *".sh"* ]]  # Check for files in ls output
}

@test "Client-Server: Command with output redirection" {
    # Start server
    ./dsh -s -i 0.0.0.0 -p 7890 &
    server_pid=$!
    
    sleep 1
    
    # Create temp directory for test
    TEST_DIR=$(mktemp -d)
    
    # Run command with output redirection
    echo "echo test_output > $TEST_DIR/test_output.txt" | ./dsh -c -i 10.246.251.11 -p 7890
    
    # Run command to cat the output file
    output=$(echo "cat $TEST_DIR/test_output.txt" | ./dsh -c -i 10.246.251.11 -p 7890)
    
    # Send stop-server command
    echo "stop-server" | ./dsh -c -i 10.246.251.11 -p 7890
    
    wait $server_pid
    
    # Check content of the file matches
    [[ "$(cat $TEST_DIR/test_output.txt 2>/dev/null)" == "test_output" ]] || 
        [[ "$output" == *"test_output"* ]]
    
    
    rm -rf $TEST_DIR
}


@test "Client-Server: Command with input redirection" {
    # Start server
    ./dsh -s -i 0.0.0.0 -p 7890 &
    server_pid=$!
    
    sleep 1
    
    # Create temp directory and input file
    TEST_DIR=$(mktemp -d)
    echo "test_input_data" > $TEST_DIR/test_input.txt
    
    # Run command with input redirection
    run bash -c "echo 'cat < $TEST_DIR/test_input.txt' | ./dsh -c -i 10.246.251.11 -p 7890"
    
    # Send stop-server command
    echo "stop-server" | ./dsh -c -i 10.246.251.11 -p 7890
    
    wait $server_pid
    
    # Check output matches input data
    [[ "$output" == *"test_input_data"* ]]
    
    rm -rf $TEST_DIR
}

@test "Client-Server: Command with pipes" {
    # Start server
    ./dsh -s -i 0.0.0.0 -p 7890 &
    server_pid=$!
    
    sleep 1
    
    # Run command with pipes
    run bash -c "echo 'ls -la | grep \".sh\" | wc -l' | ./dsh -c -i 10.246.251.11 -p 7890"
    
    # Send stop-server command
    echo "stop-server" | ./dsh -c -i 10.246.251.11 -p 7890
    
    wait $server_pid
    
    # Check output count of .sh files = 2
    [[ "$output" =~ "2" ]]
}


@test "Client-Server: Exit command from client" {
    # Start server
    ./dsh -s -i 0.0.0.0 -p 7890 &
    server_pid=$!
    
    sleep 1
    
    # Send exit command from client
    run bash -c "echo 'exit' | ./dsh -c -i 10.246.251.11 -p 7890"
    
    # Check that the server is still running
    run ps -p $server_pid
    [ "$status" -eq 0 ]
    
    # Stop-server command
    echo "stop-server" | ./dsh -c -i 10.246.251.11 -p 7890
    
    wait $server_pid
}

@test "Client-Server: Invalid command execution" {
    # Start server
    ./dsh -s -i 0.0.0.0 -p 7890 &
    server_pid=$!
    
    sleep 1
    
    # Run invalid command
    run bash -c "echo 'this_command_does_not_exist' | ./dsh -c -i 10.246.251.11 -p 7890"
    
    # Send stop-server command
    echo "stop-server" | ./dsh -c -i 10.246.251.11 -p 7890
    
    wait $server_pid
    
    # Check for error message
    [[ "$output" == *"command not found"* ]] || [[ "$output" == *"No such file"* ]]
}

@test "Client-Server: Multiple rapid commands execution" {
    # Start server
    ./dsh -s -i 0.0.0.0 -p 7890 &
    server_pid=$!
    
    sleep 1
    
    # Create script with multiple commands
    TEST_DIR=$(mktemp -d)
    COMMAND_FILE="$TEST_DIR/commands.txt"
    
    for i in {1..20}; do
        echo "echo test_command_$i" >> $COMMAND_FILE
    done
    
    # Execute multiple commands at once
    run bash -c "cat $COMMAND_FILE | ./dsh -c -i 0.0.0.0 -p 7890"
    
    # Send stop-server command
    echo "stop-server" | ./dsh -c -i 0.0.0.0 -p 7890
    
    wait $server_pid
    
    # Check if all outputs are present
    for i in {1..20}; do
        [[ "$output" == *"test_command_$i"* ]]
    done
    
    rm -rf $TEST_DIR
}

