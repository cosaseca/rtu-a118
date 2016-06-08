import SimpleHTTPServer
import SocketServer
PORT = 80

if __name__ == "__main__":
    Handler = SimpleHTTPServer.SimpleHTTPRequestHandler
    httpd = SocketServer.TCPServer(("", PORT), Handler)
    print "serving at port", PORT
    httpd.serve_forever()