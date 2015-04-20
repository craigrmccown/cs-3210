from rq import Queue, Connection, Worker

import jobs

with Connection():
    qs = Queue('topology')
    w = Worker(qs)
    w.work()
