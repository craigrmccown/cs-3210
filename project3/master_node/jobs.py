import requests


def notify_topology_addition(new_node, old_node):
    response = requests.post(build_url_from_node(old_node), new_node)
    assert response.status_code == 200

    response = requests.post(build_url_from_node(new_node), old_node)
    assert response.status_code == 200


def notify_topology_removal():
    pass


def build_url_from_node(node):
    return ''.join(['http://', node.get('ip_address'), ':', node.get('port_number'), '/topology'])