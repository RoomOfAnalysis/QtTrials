#!/usr/bin/env python

import zmq
import sys
import threading


def tprint(msg):
    """like print, but won't get newlines confused with multiple threads"""
    sys.stdout.write(msg + '\n')
    sys.stdout.flush()


class ServerTask(threading.Thread):
    """ServerTask"""

    def __init__(self):
        threading.Thread.__init__(self)

    def run(self):
        context = zmq.Context()
        frontend = context.socket(zmq.ROUTER)
        frontend.bind('tcp://*:5570')

        backend = context.socket(zmq.DEALER)
        backend.bind('inproc://backend')

        worker = ServerWorker(context)
        worker.start()

        zmq.proxy(frontend, backend)

        frontend.close()
        backend.close()
        context.term()


class ServerWorker(threading.Thread):
    """ServerWorker"""

    def __init__(self, context):
        threading.Thread.__init__(self)
        self.context = context

    def run(self):
        self.worker = self.context.socket(zmq.DEALER)
        self.worker.connect('inproc://backend')
        tprint('Worker started')
        while True:
            try:
                ident, msg = self.worker.recv_multipart()
                tprint('Worker received %s from %s' % (msg, ident))
                self.worker.send_multipart(
                    [ident, bytes("world", encoding='utf-8')])

            except Exception as inst:
                print("ERROR: type = {}, {}".format(type(inst), inst))
                pass

        worker.close()


def main():
    """main function"""
    server = ServerTask()
    server.start()

    server.join()


if __name__ == "__main__":
    main()
