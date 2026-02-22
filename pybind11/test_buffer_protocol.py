import os
import socket
from threading import Thread, Condition
from pose_module import Pose

SOCKET_PATH = "/tmp/pose_socket"

done = False
success = False
cv = Condition()

def reader_thread():
    # open the socket for reading,
    # read the data to a Pose object and verify contents
    with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as s:
        global success, done
        s.connect(SOCKET_PATH)
        p = Pose()
        # receive directly into the Pose object via the buffer protocol, without copying to an intermediate bytes object
        s.recv_into(p)
        success = p.x == 1.0 and p.y == 2.0 and p.z == 3.0
        with cv:
            done = True
            cv.notify_all()

def test_pose_buffer_protocol():
    # create a thread, wait for data to be sent via a local socket,
    # read the data to a Pose object and verify contents

    # Remove stale socket file from previous runs
    if os.path.exists(SOCKET_PATH):
        os.unlink(SOCKET_PATH)

    p = Pose()
    p.x = 1.0
    p.y = 2.0
    p.z = 3.0

    # Bind and listen BEFORE starting the reader thread to avoid race condition
    with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as s:
        s.bind(SOCKET_PATH)
        s.listen(1)

        t = Thread(target=reader_thread)
        t.start()

        conn, _ = s.accept()
        with conn:
            # send directly via the buffer protocol, without copying to an intermediate bytes object
            conn.send(p)

    # wait for the reader thread to finish
    with cv:
        while not done:
            cv.wait()
    print("Test passed:", success)

    t.join()

    # Clean up socket file
    if os.path.exists(SOCKET_PATH):
        os.unlink(SOCKET_PATH)

if __name__ == "__main__":
    test_pose_buffer_protocol()