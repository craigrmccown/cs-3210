#!/usr/bin/env bash

# usage: ./deploy.sh <port_number>

PORT_NUMBER=$1

#TODO run in daemon mode
echo "starting master node server..."
python api.py $PORT_NUMBER
echo "server started!"