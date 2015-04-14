from pymongo import MongoClient
from gridfs import GridFS
import requests


def replicate_file(node_id, file_hash_ring_id):
    mongo = MongoClient('localhost', 27017)
    db = mongo['rpfs_slave_db_' + str(node_id)]
    fs = GridFS(db)

    topology = db.topology.find()
    sorted_v_node_ids, v_node_map = build_topology_maps(topology)
    original_replica_node_id = get_next_v_node_id(file_hash_ring_id, sorted_v_node_ids)
    replica_node_id = original_replica_node_id

    while v_node_map[replica_node_id]['node_id'] == node_id:
        replica_node_id = get_next_v_node_id(replica_node_id, sorted_v_node_ids)

        if replica_node_id == original_replica_node_id:
            return

    replica_node = v_node_map.get(replica_node_id)
    f = fs.find_one({'filename': str(file_hash_ring_id)})

    if f:
        response = requests.post(build_url(replica_node, file_hash_ring_id), files={'file': (str(file_hash_ring_id), f.read(), f.content_type)})
        assert response.status_code == 200


def delete_from_replica_node(node_id, file_hash_ring_id):
    # find node who is current replica node of file
    # delete file from node

    pass


def replicate_to_new_node(node_id, new_node_id):
    # find all hash ring ids of files that now replicate to the new node
    # copy each file over
    # find all hash ring ids of files whose responsibility has been taken
    # copy each file over

    pass


def build_topology_maps(topology):
    v_node_ids = []
    v_node_map = {}

    for node in topology:
        for v_node in node.get('v_nodes'):
            v_node_ids.append(v_node.get('hash_ring_id'))
            v_node_map[v_node.get('hash_ring_id')] = node

    return sorted(v_node_ids), v_node_map


def get_next_v_node_id(hash_ring_id, sorted_v_node_ids):
    for i in range(len(sorted_v_node_ids)):
        if hash_ring_id < sorted_v_node_ids[i]:
            return sorted_v_node_ids[i - 1]

    return sorted_v_node_ids[0]


def build_url(node, hash_ring_id):
    return 'http://' + ':'.join([str(node.get('ip_address')), str(node.get('port_number'))]) + '/replicate/' + str(hash_ring_id)