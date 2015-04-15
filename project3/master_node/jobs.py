from pymongo import MongoClient
import requests


def notify_topology_addition(new_node):
    mongo = MongoClient('localhost', 27017)
    db = mongo.rpfs_master_db
    cursor = db.topology.find({'node_id': {'$ne': new_node.get('node_id')}})
    responses = []

    for node in cursor:
        node_dict = dict(node)
        del node_dict['_id']
        responses.append(requests.post(build_url_from_node(node) + '/topology', json=new_node))
        responses.append(requests.post(build_url_from_node(new_node) + '/topology', json=node_dict))

    assert_successful_responses(responses)


def notify_topology_removal(failed_node):
    mongo = MongoClient('localhost', 27017)
    db = mongo.rpfs_master_db
    db.topology.remove({'node_id': failed_node.get('node_id')})
    cursor = db.topology.find()
    responses = []

    for node in cursor:
        responses.append(requests.delete(build_url_from_node(node) + '/topology/' + str(node.get('node_id'))))

    assert_successful_responses(responses)


def build_url_from_node(node):
    return ''.join(['http://', node.get('ip_address'), ':', str(node.get('port_number'))])


def assert_successful_responses(responses):
    assert len(filter(lambda response: response.status_code != 200, responses)) == 0