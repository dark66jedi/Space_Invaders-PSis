import curses
import zmq
import scores_pb2 as proto

def main(stdscr):
    # ZMQ setup
    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    socket.connect("tcp://localhost:5557")  # Replace with your server's address
    socket.setsockopt_string(zmq.SUBSCRIBE, "")  # Subscribe to all messages

    # Clear the screen
    stdscr.clear()
    stdscr.addstr(0, 0, "Waiting for AstronautScores messages...")
    stdscr.refresh()

    while True:
        try:
            # Receive a message
            message = socket.recv()
            
            # Deserialize the Protobuf message
            scores = proto.AstronautScores()
            scores.ParseFromString(message)
            
            # Clear the screen for new data
            stdscr.clear()
            
            # Display the scores
            stdscr.addstr(0, 0, "Astronaut Scores:")
            for idx, score in enumerate(scores.scores):
                stdscr.addstr(idx + 1, 0, f"{score.name}: {score.score}")

            # Refresh the screen to show updates
            stdscr.refresh()

        except Exception as e:
            stdscr.addstr(0, 0, f"Error: {str(e)}")
            stdscr.refresh()
            break

# Run the curses application
curses.wrapper(main)
