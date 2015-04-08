import requests
import sys

master_ip_address = sys.argv[1]
master_port_number = int(sys.argv[2])
slave_node_id = int(sys.argv[3])
slave_node_port_number = int(sys.argv[4])


def confirm_registration():
    requests.post(''.join([
        'http://', master_ip_address,
        ':', str(master_port_number),
        '/confirm-registration/', str(slave_node_id),
        '/', str(slave_node_port_number)
    ]))


if __name__ == '__main__':
    confirm_registration()