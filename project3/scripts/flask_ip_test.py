from flask import Flask, Response, request
import sys
import socket


app = Flask('flask_ip_test')
lan_address = socket.gethostbyname(socket.getfqdn())


@app.route('/asdf', methods=['GET'])
def asdf():
    sys.stderr.write(str(request.remote_addr) + '\n')
    return Response(status=200)


if __name__ == '__main__':
    app.run(lan_address, 8000)
