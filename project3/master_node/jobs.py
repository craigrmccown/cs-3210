from pymongo import MongoClient
import requests


def notify_topology_addition(new_node):
    db = MongoClient.rpfs_master_db
    db.topology.remove({'node_id': new_node.get('node_id')})
    cursor = db.topology.find()
    responses = []

    for node in cursor:
        responses.append(requests.post(build_url_from_node(node) + '/topology', new_node))
        responses.append(requests.post(build_url_from_node(new_node) + '/topology', node))

    assert_successful_responses(responses)


def notify_topology_removal(failed_node):
    db = MongoClient.rpfs_master_db
    db.topology.remove({'node_id': failed_node.get('node_id')})
    cursor = db.topology.find()
    responses = []

    for node in cursor:
        responses.append(requests.delete(build_url_from_node(node) + '/topology/' + str(node.get('node_id'))))

    assert_successful_responses(responses)


def build_url_from_node(node):
    return ''.join(['http://', node.get('ip_address'), ':', node.get('port_number')])


def assert_successful_responses(responses):
    assert len(filter(lambda response: response.status_code != 200, responses)) == 0