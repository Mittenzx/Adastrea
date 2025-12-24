import socket
import json

def test_unreal_connection():
    try:
        # Connect to AdastreaDirector IPC Server
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect(('127.0.0.1', 5555))
        
        # Send ping command
        request = {'command': 'ping', 'data': {}}
        sock.sendall((json.dumps(request) + '\n').encode('utf-8'))
        
        # Receive response
        response = sock.recv(4096).decode('utf-8')
        result = json.loads(response)
        
        print(' Successfully connected to Unreal Engine!')
        print('Server Response:', result)
        
        sock.close()
        return True
        
    except ConnectionRefusedError:
        print(' Connection refused - Unreal Editor may not be running')
        print('   Start Unreal Editor with the Adastrea project to enable IPC')
        return False
    except socket.timeout:
        print(' Connection timeout - Server not responding')
        return False
    except Exception as e:
        print(f' Error: {e}')
        return False

if __name__ == '__main__':
    test_unreal_connection()
