from pymongo import MongoClient
import os
import requests
import hashlib
import array
import mimetypes
import sys


mongo = MongoClient('localhost', 27017)
db = mongo.rpfs_master_db


def notify_topology_addition(new_node):
    cursor = db.topology.find()
    responses = []

    for node in cursor:
        del node['_id']
        responses.append(requests.post(build_url_from_node(node) + '/topology', json=new_node))
        responses.append(requests.post(build_url_from_node(new_node) + '/topology', json=node))

    assert_successful_responses(responses)


def notify_topology_removal(failed_node):
    db.topology.remove({'node_id': failed_node.get('node_id')})
    cursor = db.topology.find()
    responses = []

    for node in cursor:
        responses.append(requests.delete(build_url_from_node(node) + '/topology/' + str(failed_node.get('node_id'))))

    assert_successful_responses(responses)


def write_files():
    write_path = '/tmp/rpfs/write'
    to_be_written = os.listdir(write_path)
    to_be_written = filter(lambda p: os.path.isfile(os.path.join(write_path, p)), to_be_written)

    if len(to_be_written) == 0:
        return

    topology = list(db.topology.find())

    if len(topology) == 0:
        return

    sorted_v_node_ids, v_node_map = build_topology_maps(topology)

    for path in to_be_written:
        hash_ring_id = generate_hash_ring_id(path)
        next_v_node_index = get_next_v_node_id_index(hash_ring_id, sorted_v_node_ids)
        next_node = v_node_map[sorted_v_node_ids[next_v_node_index]]

        with open(os.path.join(write_path, path), 'r') as f:
            requests.post(build_url_from_node(next_node) + '/files/' + str(hash_ring_id), files={'file': (str(hash_ring_id), f, mimetypes.guess_type(path)[0])})

        os.remove(os.path.join(write_path, path))


def read_files():
    readpath_path = '/tmp/rpfs/pyreadpath'
    read_path = '/tmp/rpfs/read'

    to_be_read = os.listdir(readpath_path)
    to_be_read = filter(lambda p: os.path.isfile(os.path.join(readpath_path, p)), to_be_read)

    if len(to_be_read) == 0:
        return

    topology = list(db.topology.find())

    if len(topology) == 0:
        return

    sorted_v_node_ids, v_node_map = build_topology_maps(topology)

    for path in to_be_read:
        hash_ring_id = generate_hash_ring_id(path)
        next_v_node_index = get_next_v_node_id_index(hash_ring_id, sorted_v_node_ids)
        next_node = v_node_map[sorted_v_node_ids[next_v_node_index]]

        response = requests.get(build_url_from_node(next_node) + '/files/' + str(hash_ring_id))

        with open(os.path.join(read_path, path), 'w') as f:
            f.write(response.content)

        os.remove(os.path.join(readpath_path, path))


def build_topology_maps(topology):
    v_node_ids = []
    v_node_map = {}

    for node in topology:
        for v_node in node.get('v_nodes'):
            v_node_ids.append(v_node.get('hash_ring_id'))
            v_node_map[v_node.get('hash_ring_id')] = node

    return sorted(v_node_ids), v_node_map


def get_next_v_node_id_index(hash_ring_id, sorted_v_node_ids):
    for i in range(len(sorted_v_node_ids)):
        if hash_ring_id < sorted_v_node_ids[i]:
            return i

    return 0


def generate_hash_ring_id(filename):
    md5 = hashlib.md5()
    md5.update(filename)
    digest = md5.hexdigest()
    byte_array = array.array('B', digest)[-4:]
    return byte_array[3] | (byte_array[2] << 8) | (byte_array[1] << 16) | (byte_array[0] << 24)


def build_url_from_node(node):
    return ''.join(['http://', node.get('ip_address'), ':', str(node.get('port_number'))])


def assert_successful_responses(responses):
    assert len(filter(lambda response: response.status_code != 200, responses)) == 0