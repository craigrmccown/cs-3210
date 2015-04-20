from pymongo import MongoClient

mongo = MongoClient('localhost', 27017)
db = mongo.rpfs_master_db
topology = list(db.topology.find())
v_nodes = []

for node in topology:
    for v_node in node.get('v_nodes'):
        v_nodes.append((v_node, node))

sorted_v_nodes = sorted(v_nodes, key=lambda v: v[0])

for v_node in sorted_v_nodes:
    print str(v_node[0].get('hash_ring_id')) + ': ' + str(v_node[1].get('node_id')) + ', ' + str(v_node[1].get('port_number'))