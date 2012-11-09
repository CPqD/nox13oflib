/*
 * Copyright (c) 2008, 2009, 2010 Nicira Networks
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OPENFLOW_NICIRA_EXT_H
#define OPENFLOW_NICIRA_EXT_H 1

#include "openflow/openflow.h"

#define NICIRA_OUI_STR "002320"

/* The following vendor extensions, proposed by Nicira Networks, are not yet
 * ready for standardization (and may never be), so they are not included in
 * openflow.h. */

#define NX_VENDOR_ID 0x00002320

enum nicira_type {
    /* Switch status request.  The request body is an ASCII string that
     * specifies a prefix of the key names to include in the output; if it is
     * the null string, then all key-value pairs are included. */
    NXT_STATUS_REQUEST,

    /* Switch status reply.  The reply body is an ASCII string of key-value
     * pairs in the form "key=value\n". */
    NXT_STATUS_REPLY,

    /* No longer used. */
    NXT_ACT_SET_CONFIG, //__OBSOLETE,
    NXT_ACT_GET_CONFIG, //__OBSOLETE,
    NXT_COMMAND_REQUEST, //__OBSOLETE,
    NXT_COMMAND_REPLY, //__OBSOLETE,
    NXT_FLOW_END_CONFIG, //__OBSOLETE,
    NXT_FLOW_END, //__OBSOLETE,
    NXT_MGMT, //__OBSOLETE,

    /* Use the high 32 bits of the cookie field as the tunnel ID in the flow
     * match. */
    NXT_TUN_ID_FROM_COOKIE,

    /* Controller role support.  The request body is struct nx_role_request.
     * The reply echos the request. */
    NXT_ROLE_REQUEST,
    NXT_ROLE_REPLY
};

struct nicira_header {
    struct ofp_header header;
    uint32_t vendor;            /* NX_VENDOR_ID. */
    uint32_t subtype;           /* One of NXT_* above. */
};
OFP_ASSERT(sizeof(struct nicira_header) == 16);

enum nx_snat_command {
    NXSC_ADD,
    NXSC_DELETE
};

/* Configuration for source-NATing */
struct nx_snat_config {
    uint8_t command;        /* One of NXSC_*. */
    uint8_t pad[3];
    uint32_t port;          /* Physical switch port. */
    uint16_t mac_timeout;   /* Time to cache MAC addresses of SNAT'd hosts
                               in seconds.  0 uses the default value. */

    /* Range of IP addresses to impersonate.  Set both values to the
     * same to support a single address.  */
    uint8_t pad2[2];
    uint32_t ip_addr_start; 
    uint32_t ip_addr_end;

    /* Range of transport ports that should be used as new source port.  A
     * value of zero, let's the switch choose.*/
    uint16_t tcp_start;
    uint16_t tcp_end;
    uint16_t udp_start;
    uint16_t udp_end;

    /* MAC address to use for ARP requests for a SNAT IP address that 
     * comes in on a different interface than 'port'.  A value of all 
     * zeros silently drops those ARP requests.  Requests that arrive 
     * on 'port' get a response with the mac address of the datapath 
     * device. */
    uint8_t mac_addr[OFP_ETH_ALEN];
    uint8_t pad3[2];
};
OFP_ASSERT(sizeof(struct nx_snat_config) == 36);

/* Action configuration.  Not all actions require separate configuration. */
struct nx_act_config {
    struct nicira_header header;
    uint16_t type;          /* One of OFPAT_* */
    uint8_t pad[2];
    union {
        struct nx_snat_config snat[0];
    };                      /* Array of action configurations.  The number 
                               is inferred from the length field in the 
                               header. */
};
OFP_ASSERT(sizeof(struct nx_act_config) == 20);


/* Action structure for NXAST_SNAT. */
struct nx_action_snat {
    uint16_t type;                  /* OFPAT_VENDOR. */
    uint16_t len;                   /* Length is 8. */
    uint32_t vendor;                /* NX_VENDOR_ID. */
    uint16_t subtype;               /* NXAST_SNAT. */
    uint8_t pad[2];
    uint32_t port;                  /* Output port--it must be previously 
                                       configured. */
};
OFP_ASSERT(sizeof(struct nx_action_snat) == 16);

/* Status bits for NXT_COMMAND_REPLY. */
enum {
    NXT_STATUS_EXITED = 1 << 31,   /* Exited normally. */
    NXT_STATUS_SIGNALED = 1 << 30, /* Exited due to signal. */
    NXT_STATUS_UNKNOWN = 1 << 29,  /* Exited for unknown reason. */
    NXT_STATUS_COREDUMP = 1 << 28, /* Exited with core dump. */
    NXT_STATUS_ERROR = 1 << 27,    /* Command could not be executed. */
    NXT_STATUS_STARTED = 1 << 26,  /* Command was started. */
    NXT_STATUS_EXITSTATUS = 0xff,  /* Exit code mask if NXT_STATUS_EXITED. */
    NXT_STATUS_TERMSIG = 0xff,     /* Signal number if NXT_STATUS_SIGNALED. */
};

/* NXT_COMMAND_REPLY. */
struct nx_command_reply {
    struct nicira_header nxh;
    uint32_t status;            /* Status bits defined above. */
    /* Followed by any number of bytes of process output. */
};
OFP_ASSERT(sizeof(struct nx_command_reply) == 20);

struct nxt_tun_id_cookie {
    struct ofp_header header;
    uint32_t vendor;            /* NX_VENDOR_ID. */
    uint32_t subtype;           /* NXT_TUN_ID_FROM_COOKIE */
    uint8_t set;                /* Nonzero to enable, zero to disable. */
    uint8_t pad[7];
};
OFP_ASSERT(sizeof(struct nxt_tun_id_cookie) == 24);

/* Configures the "role" of the sending controller.  The default role is:
 *
 *    - Other (NX_ROLE_OTHER), which allows the controller access to all
 *      OpenFlow features.
 *
 * The other possible roles are a related pair:
 *
 *    - Master (NX_ROLE_MASTER) is equivalent to Other, except that there may
 *      be at most one Master controller at a time: when a controller
 *      configures itself as Master, any existing Master is demoted to the
 *      Slave role.
 *
 *    - Slave (NX_ROLE_SLAVE) allows the controller read-only access to
 *      OpenFlow features.  In particular attempts to modify the flow table
 *      will be rejected with an OFPBRC_EPERM error.
 *
 *      Slave controllers also do not receive asynchronous messages
 *      (OFPT_PACKET_IN, OFPT_FLOW_REMOVED, OFPT_PORT_STATUS).
 */
struct nx_role_request {
    struct nicira_header nxh;
    uint32_t role;              /* One of NX_ROLE_*. */
};

enum nx_role {
    NX_ROLE_OTHER,              /* Default role, full access. */
    NX_ROLE_MASTER,             /* Full access, at most one. */
    NX_ROLE_SLAVE               /* Read-only access. */
};

enum nx_action_subtype {
    NXAST_SNAT, //__OBSOLETE,           /* No longer used. */

    /* Searches the flow table again, using a flow that is slightly modified
     * from the original lookup:
     *
     *    - The 'in_port' member of struct nx_action_resubmit is used as the
     *      flow's in_port.
     *
     *    - If NXAST_RESUBMIT is preceded by actions that affect the flow
     *      (e.g. OFPAT_SET_VLAN_VID), then the flow is updated with the new
     *      values.
     *
     * Following the lookup, the original in_port is restored.
     *
     * If the modified flow matched in the flow table, then the corresponding
     * actions are executed.  Afterward, actions following NXAST_RESUBMIT in
     * the original set of actions, if any, are executed; any changes made to
     * the packet (e.g. changes to VLAN) by secondary actions persist when
     * those actions are executed, although the original in_port is restored.
     *
     * NXAST_RESUBMIT may be used any number of times within a set of actions.
     *
     * NXAST_RESUBMIT may nest to an implementation-defined depth.  Beyond this
     * implementation-defined depth, further NXAST_RESUBMIT actions are simply
     * ignored.  (Open vSwitch 1.0.1 and earlier did not support recursion.)
     */
    NXAST_RESUBMIT,

    /* Set encapsulating tunnel ID. */
    NXAST_SET_TUNNEL,

    /* Stops processing further actions, if the packet being processed is an
     * Ethernet+IPv4 ARP packet for which the source Ethernet address inside
     * the ARP packet differs from the source Ethernet address in the Ethernet
     * header.
     *
     * This is useful because OpenFlow does not provide a way to match on the
     * Ethernet addresses inside ARP packets, so there is no other way to drop
     * spoofed ARPs other than sending every packet up to the controller. */
    NXAST_DROP_SPOOFED_ARP
};

/* Action structure for NXAST_RESUBMIT. */
struct nx_action_resubmit {
    uint16_t type;                  /* OFPAT_VENDOR. */
    uint16_t len;                   /* Length is 16. */
    uint32_t vendor;                /* NX_VENDOR_ID. */
    uint16_t subtype;               /* NXAST_RESUBMIT. */
    uint8_t pad[2];
    uint32_t in_port;               /* New in_port for checking flow table. */
};
OFP_ASSERT(sizeof(struct nx_action_resubmit) == 16);

/* Action structure for NXAST_SET_TUNNEL. */
struct nx_action_set_tunnel {
    uint16_t type;                  /* OFPAT_VENDOR. */
    uint16_t len;                   /* Length is 16. */
    uint32_t vendor;                /* NX_VENDOR_ID. */
    uint16_t subtype;               /* NXAST_SET_TUNNEL. */
    uint8_t pad[2];
    uint32_t tun_id;                /* Tunnel ID. */
};
OFP_ASSERT(sizeof(struct nx_action_set_tunnel) == 16);

/* Header for Nicira-defined actions. */
struct nx_action_header {
    uint16_t type;                  /* OFPAT_VENDOR. */
    uint16_t len;                   /* Length is 16. */
    uint32_t vendor;                /* NX_VENDOR_ID. */
    uint16_t subtype;               /* NXAST_*. */
    uint8_t pad[6];
};
OFP_ASSERT(sizeof(struct nx_action_header) == 16);

/* Wildcard for tunnel ID. */
#define NXFW_TUN_ID  (1 << 25)
#define NXFW_NW_SRC  (1 << 26)
#define NXFW_NW_DST  (1 << 27)
#define NXFW_DL_SRC  (1 << 28)
#define NXFW_DL_DST  (1 << 29)

#define NXFW_ALL NXFW_TUN_ID | NXFW_NW_SRC | NXFW_NW_DST | NXFW_DL_SRC | NXFW_DL_DST
#define OVSFW_ALL (OFPFW_ALL | NXFW_ALL)

#endif /* openflow/nicira-ext.h */
