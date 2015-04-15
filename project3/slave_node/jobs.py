from pymongo import MongoClient
from gridfs import GridFS
import requests
import math


def replicate_file(node_id, file_hash_ring_id):
    mongo = MongoClient('localhost', 27017)
    db = mongo['rpfs_slave_db_' + str(node_id)]
    fs = GridFS(db)
    topology = db.topology.find()

    replica_node = get_next_node(topology, file_hash_ring_id)

    if not replica_node:
        return

    f = fs.find_one({'filename': str(file_hash_ring_id)})

    if f:
        response = send_file(f, replica_node)
        assert response.status_code == 200


def delete_from_replica_node(node_id, file_hash_ring_id):
    mongo = MongoClient('localhost', 27017)
    db = mongo['rpfs_slave_db_' + str(node_id)]
    topology = db.topology.find()

    replica_node = get_next_node(topology, file_hash_ring_id)

    if not replica_node:
        return

    response = requests.delete(build_url(replica_node) + '/files/' + str(file_hash_ring_id))
    assert response.status_code == 200


def replicate_to_new_node(node_id, new_node_id):
    mongo = MongoClient('localhost', 27017)
    db = mongo['rpfs_slave_db_' + str(node_id)]
    topology = db.topology.find()[:]
    responses = []

    sorted_v_node_ids, v_node_map = build_topology_maps(topology)

    # replicate files to new node if it becomes the current node's replica node
    for node in topology:
        for v_node in node.get('v_nodes'):
            next_v_node_id = get_next_distinct_v_node_id(v_node.get('hash_ring_id'), sorted_v_node_ids, v_node_map)

            if not next_v_node_id:
                break

            next_node = v_node_map.get(next_v_node_id)

            if next_node.get('node_id') == new_node_id:
                previous_v_node_id = get_previous_v_node_id(v_node.get('hash_ring_id'), sorted_v_node_ids)
                pre_zero_low, pre_zero_high, post_zero_low, post_zero_high = resolve_hash_ring_bounds(previous_v_node_id, v_node.get('hash_ring_id'))

                file_cursor = db.fs.files.find({
                    '$and': [
                        {
                            'hash_ring_id': {
                                '$gt': pre_zero_low,
                                '$lte': pre_zero_high
                            }
                        },
                        {
                            'hash_ring_id': {
                                '$gt': post_zero_low,
                                '$lte': post_zero_high
                            }
                        }
                    ]
                })

                for f in file_cursor:
                    responses.append(send_file(f, next_node))

    # replicate files to the new node whose responsibility has been taken away by the new node
    for node in topology:
        for v_node in node.get('v_nodes'):
            previous_v_node_id = get_previous_v_node_id(v_node.get('hash_ring_id'), sorted_v_node_ids)
            previous_node = v_node_map.get(previous_v_node_id)

            if previous_node.get('node_id') == new_node_id:
                previous_previous_v_node_id = get_previous_distinct_v_node_id(
                    previous_v_node_id,
                    sorted_v_node_ids,
                    v_node_map
                )

                if not previous_previous_v_node_id:
                    break

                pre_zero_low, pre_zero_high, post_zero_low, post_zero_high = resolve_hash_ring_bounds(previous_previous_v_node_id, previous_v_node_id)

                file_cursor = db.fs.files.find({
                    '$and': [
                        {
                            'hash_ring_id': {
                                '$gt': pre_zero_low,
                                '$lte': pre_zero_high
                            }
                        },
                        {
                            'hash_ring_id': {
                                '$gt': post_zero_low,
                                '$lte': post_zero_high
                            }
                        }
                    ]
                })

                for f in file_cursor:
                    responses.append(send_file(f, previous_node))

    assert_successful_responses(responses)


def resolve_hash_ring_bounds(low, high):
    pre_zero_low = math.pow(2, 31) - 1
    pre_zero_high = math.pow(2, 32) - 1
    post_zero_low = 0
    post_zero_high = high

    if low > high:
        pre_zero_low = low
    else:
        post_zero_low = low

    return pre_zero_low, pre_zero_high, post_zero_low, post_zero_high


def build_topology_maps(topology):
    v_node_ids = []
    v_node_map = {}

    for node in topology:
        for v_node in node.get('v_nodes'):
            v_node_ids.append(v_node.get('hash_ring_id'))
            v_node_map[v_node.get('hash_ring_id')] = node

    return sorted(v_node_ids), v_node_map


def get_next_node(topology, hash_ring_id):
    sorted_v_node_ids, v_node_map = build_topology_maps(topology)
    immediate_v_node_id = get_next_v_node_id(hash_ring_id, sorted_v_node_ids)
    return v_node_map.get(get_next_distinct_v_node_id(immediate_v_node_id, sorted_v_node_ids, v_node_map))


def get_previous_node(topology, hash_ring_id):
    sorted_v_node_ids, v_node_map = build_topology_maps(topology)
    immediate_v_node_id = get_previous_v_node_id(hash_ring_id, sorted_v_node_ids)
    return v_node_map.get(get_previous_distinct_v_node_id(immediate_v_node_id, sorted_v_node_ids, v_node_map))


def get_next_distinct_v_node_id(hash_ring_id, sorted_v_node_ids, v_node_map):
    starting_node = v_node_map.get(hash_ring_id)

    assert starting_node

    distinct_v_node_id = get_next_v_node_id(hash_ring_id, sorted_v_node_ids)

    while v_node_map[distinct_v_node_id]['node_id'] == starting_node['node_id']:
        distinct_v_node_id = get_next_v_node_id(distinct_v_node_id, sorted_v_node_ids)

        if distinct_v_node_id == hash_ring_id:
            return

    return distinct_v_node_id


def get_previous_distinct_v_node_id(v_node_hash_ring_id, sorted_v_node_ids, v_node_map):
    starting_node = v_node_map.get(v_node_hash_ring_id)

    assert starting_node

    distinct_v_node_id = get_previous_v_node_id(v_node_hash_ring_id, sorted_v_node_ids)

    while v_node_map[distinct_v_node_id]['node_id'] == starting_node['node_id']:
        distinct_v_node_id = get_previous_v_node_id(distinct_v_node_id, sorted_v_node_ids)

        if distinct_v_node_id == v_node_hash_ring_id:
            return

    return distinct_v_node_id


def get_next_v_node_id(hash_ring_id, sorted_v_node_ids):
    for i in range(len(sorted_v_node_ids)):
        if hash_ring_id > sorted_v_node_ids[i]:
            return sorted_v_node_ids[i - 1]

    return sorted_v_node_ids[0]


def get_previous_v_node_id(hash_ring_id, sorted_v_node_ids):
    for i in range(len(sorted_v_node_ids)):
        if hash_ring_id < sorted_v_node_ids[i]:
            return sorted_v_node_ids[i - 1]

    return sorted_v_node_ids[-1]


def send_file(f, node):
    return requests.post(
        build_url(node) + '/replicate/' + str(f.hash_ring_id),
        files={'file': (str(f.hash_ring_id), f.read(), f.content_type)}
    )


def build_url(node):
    return 'http://' + ':'.join([str(node.get('ip_address')), str(node.get('port_number'))])


def assert_successful_responses(responses):
    assert len(filter(lambda response: response.status_code != 200, responses)) == 0