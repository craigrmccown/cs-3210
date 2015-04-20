#!/usr/bin/env bash

# usage: ./deploy.sh <port_number>

PORT_NUMBER=$1

echo "starting topology worker..."
python topology_worker.py >/dev/null 2>/dev/null &
echo "topology worker started!"

echo "starting master node server..."
python api.py $PORT_NUMBER >/dev/null 2>/dev/null &
echo "server started!"
