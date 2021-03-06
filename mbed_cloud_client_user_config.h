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


#ifndef MBED_CLOUD_CLIENT_USER_CONFIG_H
#define MBED_CLOUD_CLIENT_USER_CONFIG_H

#define MBED_CLOUD_CLIENT_ENDPOINT_TYPE         "E2E-Dev"
#define MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP
#define MBED_CLOUD_CLIENT_LIFETIME              3600

#define SN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE       1024

/* set flag to enable update support in mbed Cloud client */
#define MBED_CLOUD_CLIENT_SUPPORT_UPDATE

/* set download buffer size in bytes (min. 1024 bytes) */

/* Use larger buffers in Linux */
#define MBED_CLOUD_CLIENT_UPDATE_BUFFER          2048 * 16

/* Developer flags for Update feature */
#define MBED_CONF_APP_DEVELOPER_MODE 1
#define MBED_CLOUD_DEV_UPDATE_CERT
#define MBED_CLOUD_DEV_UPDATE_ID

#endif /* MBED_CLOUD_CLIENT_USER_CONFIG_H */
