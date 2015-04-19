#!/usr/bin/env bash

# usage: ./deploy.sh <port_number>

PORT_NUMBER=$1

echo "creating tmp directories..."
mkdir -p /tmp/rpfs/write
mkdir -p /tmp/rpfs/pyreadpath
mkdir -p /tmp/rpfs/read
echo "directories created!"

#TODO run in daemon mode
echo "starting master node server..."
python api.py $PORT_NUMBER
echo "server started!"

#TODO run topology worker
echo "starting replication worker..."
echo "replication worker started!"