#!/usr/bin/env python

"""A simple python script template.
"""

from __future__ import print_function
import os
import sys
import argparse
from mbed_cloud import ConnectAPI

def _subscription_handler(device_id, path, value):
    print("Device: %s, Resoure path: %s, Current value: %r" % (device_id, path, value))

def main(arguments):

    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("api_key", help="Mbed Cloud API Key")
    parser.add_argument("device_id", help="Device ID")
    parser.add_argument("-H", "--host", help="Mbed Cloud API URL", default="https://api.us-east-1.mbedcloud.com")

    args = parser.parse_args(arguments)
    connect_api = ConnectAPI({
        "api_key": args.api_key,
        "host": args.host
    })

    connect_api.start_notifications()
    connect_api.add_resource_subscription_async(args.device_id, "/3200/0/5501", _subscription_handler)

    while True:
        pass



if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
