from rq import Queue, Connection, Worker
import jobs

with Connection():
    qs = Queue('replication')
    w = Worker(qs)
    w.work()
