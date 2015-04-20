from flask import Flask, Response, request
import sys


app = Flask('flask_ip_test')


@app.route('/asdf', methods=['GET'])
def asdf():
    sys.stderr.write(str(request.remote_addr))
    return Response(status=200)