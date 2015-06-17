#!/usr/bin/env python3

import time
import socket
import struct
import selectors
import itertools
import array
import sys
from collections import namedtuple
from datetime import datetime

import h5py

PORT = 12345
HOSTNAME = 'localhost'
TASK_HANDLE = 1
NUM_CHANNELS = 64
SAMPLE_RATE = 10000
BUFFER_SIZE = 4096

# Message types
INIT_EXPERIMENT = 0x00
EXPT_PARAMS_REQ = 0x01
EXPT_PARAMS = 0x02
CHECK_READY = 0x03
READY_STATUS = 0x04
START_EXPT = 0x05
DATA_CHUNK = 0x06
CLOSE = 0x07
ERROR_MSG = 0x08

init_msg = namedtuple('init_msg', ['msg_type', 'msg_size', 'trigger_length',
        'trigger', 'expt_length', 'adc_range', 'block_size'])
params_msg = namedtuple('params_msg', ['msg_type', 'msg_size', 'nseconds', 'nsamples',
        'sample_rate', 'adc_range', 'adc_resolution', 'block_size', 'nchannels',
        'trigger_length', 'trigger', 'date_length', 'date'])
err_msg = namedtuple('err_msg', ['msg_type', 'err_msg_size', 'err_msg'])
task = namedtuple('task', ['handle', 'nchannels', 'length', 'nsamples', 
        'block_size', 'sample_rate', 'adc_range', 'adc_resolution', 
        'trigger_length', 'trigger', 'date_length', 'date'])

def read_msg_header(data):
    """Return the message type and message size from the
    bytes of a message.
    """
    return struct.unpack('>II', data[:8])

def parse_init_msg(data):
    """Parse the given initialization message from its bytes."""
    msg = init_msg
    offset = 0
    msg.msg_type, msg.msg_size, msg.trigger_length = struct.unpack('>III', data[:12])
    offset += 12
    msg.trigger = data[offset : offset + msg.trigger_length].decode('ascii')
    offset += msg.trigger_length
    msg.expt_length, msg.adc_range, msg.block_size = struct.unpack(
            '>ffI', data[offset:])
    return msg

def recv_init_msg(client):
    # Set up polling
    client.settimeout(0.0)
    sel = selectors.DefaultSelector()
    sel.register(client, selectors.EVENT_READ)
    print("Waiting for init msg")
    while True:
        events = sel.select()
        for (key, event) in events:
            if client == key.fileobj:
                buf = client.recv(BUFFER_SIZE)
                if buf:
                    msg_type, msg_size = read_msg_header(buf)
                    if msg_type == INIT_EXPERIMENT:
                        sel.unregister(client)
                        return parse_init_msg(buf)
                else:
                    raise ConnectionResetError # If client disconnects

def create_task(init_msg):
    t = task(TASK_HANDLE, NUM_CHANNELS, init_msg.expt_length,
            int(init_msg.expt_length * SAMPLE_RATE), init_msg.block_size, SAMPLE_RATE,
            init_msg.adc_range, (init_msg.adc_range / 2) / (2**16), 
            init_msg.trigger_length, init_msg.trigger, 0,
            datetime.strftime(datetime.now(), '%Y-%m-%d_%H-%M-%S'))
    t = t._replace(date_length=len(t.date))
    print("Task initialized:")
    for field in t._fields:
        print(" {0}: {1}".format(field, getattr(t, field)))
    return t

def serve_file_data(task, client, f):
    nsamples = 0
    while nsamples < task.nsamples:
        try:
            buf = client.recv(BUFFER_SIZE)
            if len(buf) == 0:
                print("Client disconnected, closing task")
                return
        except BlockingIOError:
            pass
        tmp = array.array('h', 
                f['data'][:, nsamples : nsamples + task.block_size].tobytes())
        try:
            client.send(tmp.tobytes())
        except BlockingIOError:
            print("Client would block, closing task")
            return
        nsamples += task.block_size
        print("{0} total samples sent".format(nsamples))
        time.sleep(task.block_size / task.sample_rate)

    print("Task completed, {0} total samples sent ({1} bytes)".format(
            nsamples, nsamples * task.nchannels * 2))

def serve_data(task, client):
    print("Recieved START_EXPT message, serving data to client")
    # Group by sample
    # data_base = itertools.chain.from_iterable(list([each] * task.nchannels for
    #        each in range(task.block_size)))

    # Group by channel
    data_base = itertools.chain.from_iterable(
            [[chan] * task.block_size
            for chan in range(task.nchannels)])
    data = array.array('h', data_base)
    data_bytes = data.tobytes()
    #hdr = int(DATA_CHUNK).to_bytes(4, 'big') + int(len(data) + 14).to_bytes(4, 'big')
    #data_hdr = int(NUM_CHANNELS).to_bytes(2, 'big') + int(task.block_size).to_bytes(4, 'big')
    #msg = hdr + data_hdr + data.tobytes()

    # Send random datas
    nsamples = 0
    while nsamples < task.nsamples:
        try: 
            buf = client.recv(BUFFER_SIZE)
            if len(buf) == 0:
                print("Client disconnected, closing task");
                return
        except BlockingIOError:
            pass
        #client.send(msg)
        try:
            client.send(data_bytes)
        except BlockingIOError:
            print("Client would block, closing task")
            return
        nsamples += task.block_size
        print("{0} total samples sent".format(nsamples))
        time.sleep(task.block_size / task.sample_rate)

    print("Task completed, {0} total samples sent ({1} bytes)".format(
            nsamples, nsamples * task.nchannels * 2))

def serve(task, client, f=None):
    sel = selectors.DefaultSelector()
    sel.register(client, selectors.EVENT_READ)
    while True:
        events = sel.select()
        if events:
            key, event = events[0]
            if client == key.fileobj:
                buf = client.recv(BUFFER_SIZE)
                if buf:
                    msg_type, msg_size = read_msg_header(buf)
                    if msg_type == EXPT_PARAMS_REQ:
                        send_expt_params(task, client)
                    elif msg_type == START_EXPT:
                        break
                else:
                    return # If client disconnects

    sel.unregister(client)
    del sel
    if f is None:
        serve_data(task, client)
    else:
        serve_file_data(task, client, f)

def send_expt_params(task, client):
    msg = bytes()
    msg_size = 4 * 10 + 2 + task.trigger_length + task.date_length
    msg += struct.pack('>IIfIfffIhI', EXPT_PARAMS, msg_size, task.length,
            task.nsamples, task.sample_rate, task.adc_range, task.adc_resolution,
            task.block_size, task.nchannels, task.trigger_length)
    msg += task.trigger.encode('ascii')
    msg += task.date_length.to_bytes(4, 'big')
    msg += task.date.encode('ascii')
    client.send(msg)

def init_server():
    print("Initializing fake daqsrv...")
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind((HOSTNAME, PORT))
    server.listen(1)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
    print("Listening for clients")
    return server

def accept_client(server):
    client, addr = server.accept()
    print("Accepted client at {0}:{1}".format(*addr))
    return client

def cleanup(client):
    while True:
        try:
            buf = client.recv(BUFFER_SIZE)
            if len(buf) == 0:
                break
        except BlockingIOError:
            pass
        except ConnectionResetError:
            break
    try:
        client.shutdown(socket.SHUT_RDWR)
        client.close()
    except:
        pass

def serve_fake_data():
    server = init_server()
    while True:
        try:
            client = accept_client(server)
            init_msg = recv_init_msg(client)
            task = create_task(init_msg)
            serve(task, client, None)

        except ConnectionResetError:
            cleanup(client)

    server.shutdown(socket.SHUT_RDWR)
    server.close()

def serve_from_file(filename):
    server = init_server()
    while True:
        try:
            f = h5py.File(filename, 'r')
            client = accept_client(server)
            init_msg = recv_init_msg(client)
            task = create_task(init_msg)
            serve(task, client, f)

        except ConnectionResetError:
            cleanup(client)
    server.shutdown(socket.SHUT_RDWR)
    server.close()
    f.close()

def main():
    try:
        if len(sys.argv) > 1:
            serve_from_file(sys.argv[1])
        else:
            serve_fake_data()
    except KeyboardInterrupt:
        pass

if __name__ == '__main__':
    main()
