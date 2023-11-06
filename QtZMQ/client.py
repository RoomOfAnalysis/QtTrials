#!/usr/bin/env python
import zmq
import sys
import threading

def tprint(msg):
    """like print, but won't get newlines confused with multiple threads"""
    sys.stdout.write(msg + '\n')
    sys.stdout.flush()

class ClientTask(threading.Thread):
    """ClientTask"""
    def __init__(self, id):
        self.id = id
        threading.Thread.__init__ (self)

    def run(self):
        context = zmq.Context()
        socket = context.socket(zmq.DEALER)
        identity = u'worker-%d' % self.id
        socket.identity = identity.encode('ascii')
        socket.connect('tcp://localhost:5570')
        print('Client %s started' % (identity))
        poll = zmq.Poller()
        poll.register(socket, zmq.POLLIN)
        reqs = 0
        while True:
            reqs = reqs + 1
            #print('Req #%d sent..' % (reqs))
            cmd = input("--> ")
            if cmd == 'exit': break

            #socket.send_string(u'request #%d' % (reqs))
            socket.send_string(cmd)
            
            sockets = dict(poll.poll(1000))
            if socket in sockets:
              msg = socket.recv()
              tprint('Client %s received: %s' % (identity, msg))

        socket.close()
        context.term()


def main():
    """main function"""
    client = ClientTask(0)
    client.start()
    client.join()


if __name__ == "__main__":
    main()
