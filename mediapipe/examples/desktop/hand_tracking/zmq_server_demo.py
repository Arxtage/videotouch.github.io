import numpy as np
import time
import zmq


ADDRESS = "tcp://127.0.0.1:7000"
context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind(ADDRESS)


if __name__ == '__main__':
    print(f'Waiting for the message on `{ADDRESS}`...')
    
    message = ''
    state_dict = {'info': []}
    
    while (True):
        message = socket.recv().decode('UTF-8')
        
        if message == 'EOQ': break
         
        try:
            record = message.strip().split('\n')
            
            # landmarks (single hand)
            global_landmarks = np.array([
                [float(x) for x in row[1:-1].strip('[]').split(' ')] 
                for row in record[2:23]
            ])
            
            # scaled landmarks (single hand)
            local_landmarks = np.array([
                [float(x) for x in row[1:-1].strip('[]').split(' ')] 
                for row in record[23:-2]
            ])
            
            # "rect square: N"
            rect_square = float(record[-1].strip().split()[-1])
            
            state_dict['info'].append({
                'frame_num': int(record[0]), 
                'time_ms': int(record[1]),
                'global_landmarks': global_landmarks.tolist(),
                'local_landmarks': local_landmarks.tolist(),
                'gesture': record[-2],
                'rect': rect_square
            })
            
            print(state_dict['info'][-1]['frame_num'])
            print(state_dict['info'][-1]['time_ms'])
            print(state_dict['info'][-1]['gesture'])
            print(state_dict['info'][-1]['rect'])
        except ValueError:
            print("Right hand is probably out of the webcam stream")
        
        new_message = f"Got {message}"
        
        socket.send(new_message.encode())
    
    socket.close()
    
    print("Communication is ended.")
