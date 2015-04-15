from flask import Flask, Response, jsonify
from pymongo import MongoClient
from redis import Redis
from rq import Queue
import requests
import threading
import sys
import math
import random
import jobs


port_number = int(sys.argv[1])
app = Flask('rpfs_master_api')
mongo = MongoClient('127.0.0.1', 27017)
db = mongo.rpfs_master_db
topology_queue = Queue('topology', connection=Redis(host='localhost', port=6379))


@app.route('/register', methods=['POST'])
def register():
    cursor = db.topology.find()
    topology = []
    current_node_ids = []

    for node in cursor:
        del node['_id']
        topology.append(node)
        current_node_ids.append(node.get('node_id'))

    new_node_id = generate_unique_node_id(current_node_ids)

    return jsonify({'node_id': new_node_id, 'topology': topology})


@app.route('/confirm-registration/<slave_node_id>/<slave_node_port_number>', methods=['POST'])
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

    existing = db.topology.find_one({'node_id': int(slave_node_id)})

    if not existing:
        topology_queue.enqueue(jobs.notify_topology_addition, new_node)
        db.topology.insert(new_node)

    return Response(status=200)


def generate_unique_node_id(current_node_ids):
    new_node_id = random.randint(0, 10000)

    while new_node_id in current_node_ids:
        new_node_id = random.randint(0, 10000)

    return new_node_id


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


def check_heartbeats():
    threading.Timer(1, check_heartbeats).start()

    cursor = db.topology.find()

    for node in cursor:
        try:
            response = requests.get(''.join(['http://', node.get('ip_address'), ':', str(node.get('port_number')), '/heartbeat']), timeout=2)
            assert response.status_code == 200
        except (requests.ConnectionError, requests.Timeout, AssertionError):
            topology_queue.enqueue(jobs.notify_topology_removal, node)


if __name__ == '__main__':
    check_heartbeats()
    app.run('127.0.0.1', port_number)
