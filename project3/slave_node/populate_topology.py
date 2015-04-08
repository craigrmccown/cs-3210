from pymongo import MongoClient
import random
import math

mongo = MongoClient('localhost', 27017)
addresses = [
    (12345, '127.0.0.1', 5000),
    (23456, '127.0.0.1', 5001),
    (34567, '127.0.0.1', 5002),
    (45678, '127.0.0.1', 5003)
]


def insert_topology_data():
    for i in range(len(addresses)):
        db = mongo['rpfs_slave_db_' + str(addresses[i][0])]
        db.drop_collection('topology')

        for j in range(len(addresses)):
            db.topology.insert(generate_topology_member(*addresses[j]))


def generate_topology_member(node_id, ip_address, port_number):
    v_nodes = []

    for i in range(50):
        v_nodes.append({'hash_ring_id': random.randint(0, math.pow(2, 31) - 1)})

    return {
        'node_id': node_id,
        'ip_address': ip_address,
        'port_number': port_number,
        'v_nodes': v_nodes
    }

if __name__ == '__main__':
    insert_topology_data()