import zmq
import curses
import scores_pb2  # Import the generated Protobuf module

def receive_score_update(stdscr):
    # Initialize curses settings
    curses.curs_set(0)  # Hide the cursor
    stdscr.clear()

    # Set up ZeroMQ subscriber
    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    socket.connect("tcp://localhost:5557")  # Same port as the publisher
    socket.setsockopt_string(zmq.SUBSCRIBE, "")  # Subscribe to all topics

    stdscr.addstr(0, 0, "Listening for updates...", curses.A_BOLD)
    stdscr.refresh()

    high_scores = []
    while True:
        # Receive the serialized Protobuf message
        message = socket.recv()
        
        # Deserialize the Protobuf message
        new_score = scores_pb2.AstronautScore()
        new_score.ParseFromString(message)
        
        # Skip if the score is invalid
        if new_score.ch == "":
            continue
        
        # Check if the astronaut already exists in high_scores
        if not any(score["ch"] == new_score.ch for score in high_scores):
            # Create a dictionary and append to high_scores
            new = {"ch": new_score.ch, "score": new_score.score}
            high_scores.append(new)
        else:
            # Update the score if the astronaut already exists
            for score in high_scores:
                if score["ch"] == new_score.ch:
                    score["score"] = new_score.score
                    break

        # Sort high_scores by score in descending order
        high_scores.sort(key=lambda x: x["score"], reverse=True)

        # Display high scores using curses
        stdscr.clear()
        stdscr.addstr(0, 0, "Astronaut High Scores", curses.A_BOLD | curses.A_UNDERLINE)
        for i, score in enumerate(high_scores, start=1):
            stdscr.addstr(i, 0, f"{i}. {score['ch']}: {score['score']}")
        stdscr.refresh()

if __name__ == "__main__":
    curses.wrapper(receive_score_update)
