from pymongo import MongoClient
import requests
import sys


master_ip_address = sys.argv[1]
master_port_number = int(sys.argv[2])
mongo = MongoClient('127.0.0.1', 27017)


def register():
    registration = requests.post(''.join(['http://', master_ip_address, ':', master_port_number, '/register']))
    assert registration.status_code == 200

    response_body = registration.json()
    slave_node_id = response_body.get('node_id')
    topology = response_body.get('topology')

    db = mongo['rpfs_slave_db_' + str(slave_node_id)]
    db.topology.drop()

    for node in topology:
        db.topology.insert(node)

    return slave_node_id


if __name__ == '__main__':
    print register()