from flask import Flask, Response
from pymongo import MongoClient
from redis import Redis
from rq import Queue
import sys
import math
import random
import jobs


port_number = sys.argv(1)
app = Flask('rpfs_master_api')
mongo = MongoClient('127.0.0.1', 27017)
db = mongo.rpfs_master_db
topology_queue = replication_queue = Queue('topology', connection=Redis(host='localhost', port=6379))


@app.route('/register', methods=['POST'])
def register():
    cursor = db.topology.find()
    topology = []
    current_node_ids = []

    for node in cursor:
        topology.append(node)
        current_node_ids.append(node.get('node_id'))

    new_node_id = random.randint(0, 10000)

    while new_node_id in current_node_ids:
        new_node_id = random.randint(0, 10000)

    return Response({'node_id': new_node_id, 'topology': topology})


@app.route('/confirm-registration/<slave_node_id>/<slave_node_port_number')
def confirm_registration(slave_node_id, slave_node_port_number):
    num_v_nodes = 50
    cursor = db.topology.find()
    current_v_node_ids = []

    for node in cursor:
        for v_node in node.get('v_nodes'):
            current_v_node_ids.append(v_node.get('hash_ring_id'))

    new_node = {
        'node_id': int(slave_node_id),
        'ip_address': '127.0.0.1',
        'port_number': int(slave_node_port_number),
        'v_nodes': generate_unique_v_nodes(num_v_nodes, current_v_node_ids)
    }

    cursor.rewind()

    for node in cursor:
        topology_queue.enqueue(jobs.notify_topology_change, new_node, node)

    db.topology.insert(new_node)


def generate_unique_v_nodes(num_v_nodes, current_v_node_ids):
    new_v_nodes = generate_random_v_nodes(num_v_nodes)
    new_v_nodes = filter(lambda v: v.get('hash_ring_id') not in current_v_node_ids, new_v_nodes)

    while len(new_v_nodes) < num_v_nodes:
        new_v_nodes.append(generate_random_v_nodes(num_v_nodes - len(new_v_nodes)))
        new_v_nodes = filter(lambda v: v.get('hash_ring_id') not in current_v_node_ids, new_v_nodes)

    return new_v_nodes


def generate_random_v_nodes(num_v_nodes):
    v_nodes = []
    max_value = math.pow(2, 31) - 1

    for i in range(num_v_nodes):
        v_nodes.append({'hash_ring_id': random.randint(0, max_value)})

    return v_nodes


if __name__ == '__main__':
    app.run('127.0.0.1', port_number)