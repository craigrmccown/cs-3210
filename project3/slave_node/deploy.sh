#!/usr/bin/env bash

# usage: ./deploy.sh <master_node_ip_address> <master_node_port_number> <slave_port_number>

MASTER_IP_ADDRESS=$1
MASTER_PORT_NUMBER=$2
SLAVE_PORT_NUMBER=$3

# register with master node
echo "registering with master node..."
SLAVE_NODE_ID=$(python register.py $MASTER_IP_ADDRESS $MASTER_PORT_NUMBER)
echo "registered slave node id with master node: $SLAVE_NODE_ID!"

echo "starting slave node server..."
python api.py $SLAVE_NODE_ID $SLAVE_PORT_NUMBER >/dev/null 2>/dev/null &
echo "server started!"

echo "confirming registration with master..."
sleep 3
python confirm_registration.py $MASTER_IP_ADDRESS $MASTER_PORT_NUMBER $SLAVE_NODE_ID $SLAVE_PORT_NUMBER
echo "registration confirmed!"