from flask import Flask, Response, request
from pymongo import MongoClient
from gridfs import GridFS
from redis import Redis
from rq import Queue
import jobs
import sys


node_id = int(sys.argv[1])
port_number = int(sys.argv[2])
app = Flask('rpfs_slave_api_' + str(node_id))
mongo = MongoClient('localhost', 27017)
db = mongo['rpfs_slave_db_' + str(node_id)]
fs = GridFS(db)
replication_queue = Queue('replication', connection=Redis(host='localhost', port=6379))


@app.route('/heartbeat', methods=['GET'])
def heartbeat():
    return Response(status=200)


@app.route('/replicate/<file_hash_ring_id>', methods=['POST'])
def replicate_put_file(file_hash_ring_id):
    return put_file(file_hash_ring_id)


@app.route('/replicate/<file_hash_ring_id>', methods=['DELETE'])
def replicate_delete_file(file_hash_ring_id):
    return remove_file(file_hash_ring_id)


@app.route('/files/<file_hash_ring_id>', methods=['POST'])
def upload_file(file_hash_ring_id):
    response = put_file(file_hash_ring_id)
    replication_queue.enqueue(jobs.replicate_file, node_id, int(file_hash_ring_id))
    return response


@app.route('/files/<file_hash_ring_id>', methods=['GET'])
def download_file(file_hash_ring_id):
    f = fs.find_one({'filename': file_hash_ring_id})

    if f:
        response = Response(response=f.read())

        response.headers['Content-Type'] = f.content_type
        response.headers['Cache-Control'] = 'no-cache, no-store, must-revalidate'
        response.headers['Pragma'] = 'no-cache'
        response.headers['Expires'] = 0

        return response
    else:
        return Response(status=404)


@app.route('/files/<file_hash_ring_id>', methods=['DELETE'])
def delete_file(file_hash_ring_id):
    response = remove_file(file_hash_ring_id)
    if response.status_code == 200:
        replication_queue.enqueue(jobs.delete_from_replica_node, node_id, int(file_hash_ring_id))


@app.route('/topology', methods=['POST'])
def add_node_to_topology():
    new_node = request.get_json(force=True)
    existing = db.topology.find_one({'node_id': new_node.get('node_id')})

    if not existing:
        replication_queue.enqueue(jobs.replicate_to_new_node, node_id, new_node.get('node_id'))
        db.topology.insert(new_node)

    return Response(status=200)


@app.route('/topology/<failed_node_id>', methods=['DELETE'])
def remove_node_from_topology(failed_node_id):
    failed_node = db.topology.find_one({'node_id': int(failed_node_id)})

    if not failed_node:
        return Response(status=404)

    db.topology.remove({'node_id': int(failed_node_id)})
    replication_queue.enqueue(jobs.recover_from_failed_node, node_id, failed_node.get('v_nodes'))

    return Response(status=200)


def put_file(file_hash_ring_id):
    f = request.files.get('file')

    if f:
        existing = db.fs.files.find_one({'filename': file_hash_ring_id})

        if existing:
            fs.delete(existing['_id'])

        fs.put(f, filename=file_hash_ring_id, hash_ring_id=int(file_hash_ring_id), content_type=f.headers.get('Content-Type'))

        return Response(status=200)
    else:
        return Response(status=400)


def remove_file(file_hash_ring_id):
    f = db.fs.files.find_one({'filename': file_hash_ring_id})

    if f:
        fs.delete(f['_id'])
        return Response(status=200)
    else:
        return Response(status=404)


if __name__ == '__main__':
    app.run('127.0.0.1', port_number)