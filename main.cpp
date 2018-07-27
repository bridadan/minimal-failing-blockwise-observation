// ----------------------------------------------------------------------------
// Copyright 2016-2017 ARM Ltd.
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------

#include "mbed.h"

// Storage, network, and logging libraries
#include "storage-selector/storage-selector.h"
#include "easy-connect/easy-connect.h"
#include "mbed-trace/mbed_trace.h"

// Mbed Cloud Client components
#include "mbed-cloud-client/MbedCloudClient.h"
#include "factory_configurator_client.h"
#include "mbed-trace-helper.h"


#define BIG_DATA_LENGTH 2048

// Global instances
Timeout timeout;
DigitalOut led(LED1, 1);

// Global pointers
MbedCloudClient* cloud_client;
void *network_interface = NULL;

// Global state
static bool registered = false;
static volatile bool new_collect = false;

static char big_data[BIG_DATA_LENGTH];

// Timer callback
void new_collect_occurred(void) {
    new_collect = true;
}

// Mbed Cloud Client registration callback
void on_registered() {
    printf("Client registered\n");

    const ConnectorClientEndpointInfo* info = cloud_client->endpoint_info();
    printf("Device Id: %s\n", info->internal_endpoint_name.c_str());
    registered = true;
}

// Mbed Cloud Client unregistration callback
void on_unregistered() {
    printf("on_unregistered\n");
}

// Mbed Cloud Client error callback
void on_error(int err) {
    printf("on_error %i\n", err);
}

// Main init function
int init() {
    cloud_client = new MbedCloudClient();

    // Connect to network
    while (network_interface == NULL) {
        network_interface = easy_connect(true);
    }

    // Initialize storage
    BlockDevice* bd =  storage_selector();
    bd->init();
    FileSystem* fs = filesystem_selector();

    // Reformat storage if not formatted
    int status = fs->unmount();
    if (status != 0) {
        printf("fs unmount fail %d. formatting fs\n", status);

        status = fs->reformat(bd);
        if (status != 0)
        {
            printf("fs format failed.");
            return -1;
        }
    }

    // Mount the file system
    status = fs->mount(bd);
    if (status != 0) {
        printf("fs mount fail %d.\n", status);
        return -1;
    }

    // Initialize logging
    if(!mbed_trace_helper_create_mutex()) {
        printf("ERROR - Mutex creation for mbed_trace failed!\n");
        return 1;
    }

    mbed_trace_init();
    mbed_trace_mutex_wait_function_set(mbed_trace_helper_mutex_wait);
    mbed_trace_mutex_release_function_set(mbed_trace_helper_mutex_release);

    // Initialize factory configurator client for storing credentials to config
    fcc_status_e fcc_status = fcc_init();
    if(fcc_status != FCC_STATUS_SUCCESS) {
        printf("fcc_init failed with status %d! - exit\n", status);
        return 1;
    }

    // Check configs are stored correctly
    fcc_status = fcc_verify_device_configured_4mbed_cloud();
    if (fcc_status != FCC_STATUS_SUCCESS) {
        printf("device not configured for cloud resets storage to an empty state. %d\n", fcc_status);

        // Format storage
        status = fs->reformat(bd);
        if (status != 0) {
            printf("fs format failed.");
            return -1;
        }

        // Reset fcc storage
        fcc_status = fcc_storage_delete();
        if (fcc_status != FCC_STATUS_SUCCESS) {
            printf("Failed to delete storage - %d\n", fcc_status);
            return -1;
        }
    }

    // Store credentials to config
    fcc_status = fcc_developer_flow();
    if(fcc_status == FCC_STATUS_KCM_FILE_EXIST_ERROR) {
        printf("Developer credentials already exists\n");
    } else if(fcc_status != FCC_STATUS_SUCCESS) {
        printf("Failed to load developer credentials - exit\n");
        return 1;
    }

    return 0;
}

int main(void) {
    // Start the network interface, init the storage, and create the Mbed Cloud Client
    int result = init();
    if (result) {
        printf("Failed to init device, exiting.\r\n");
        return result;
    }

    // create object 3200
    M2MObject* metric_object = M2MInterfaceFactory::create_object("3200");

    // create instance /3200/0
    M2MObjectInstance* collect_instance = metric_object->create_object_instance((uint16_t)0);

    // Create resource for collect count 3200/0/5501
    M2MResource* collect_res = collect_instance->create_dynamic_resource(
                                "5501", "collections", M2MResourceInstance::OPAQUE, true);
    if (collect_res) {
        collect_res->set_operation(M2MBase::GET_ALLOWED);
        collect_res->set_value(0);
    } else {
        printf("Failed to create collect resource\r\n");
        return 1;
    }

    // Add object to cloud client
    M2MObjectList obj_list;
    obj_list.push_back(metric_object);
    cloud_client->add_objects(obj_list);

    // register with mbed cloud
    cloud_client->on_registered(on_registered);
    cloud_client->on_unregistered(on_unregistered);
    cloud_client->on_error(on_error);
    cloud_client->setup(network_interface);



    int dh_update_result = 0;
    uint32_t collect_count = 0;
    uint32_t collect_interval = 5;
    while(1) {
        if (registered) {
            break;
        }
        wait(1);
    }

    new_collect = true;
    printf("################################################\r\n");
    printf("Subscribe now! Sending observation in 20 seconds\r\n");
    printf("################################################\r\n");
    led = 0;
    wait(20);
    led = 1;

    while(1) {
        if (new_collect) {
            new_collect = false;

            printf("Sending observation #%lu\r\n", collect_count);
            memset(big_data, 'a' + (new_collect % 26), BIG_DATA_LENGTH);
            collect_res->set_value((uint8_t*)big_data, BIG_DATA_LENGTH);
            collect_count++;

            // Setup the timeout again!
            timeout.attach(&new_collect_occurred, collect_interval);
        }

        wait(0.2);
    }

    return 0;
}
