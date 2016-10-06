/*********************************************************************
 * ConfD Subscriber intro example
 * Implements a configuration data provider
 *
 * (C) 2005-2007 Tail-f Systems
 * Permission to use this code as a starting point hereby granted
 *
 * See the README file for more information
 ********************************************************************/
#include "nsf-secu-controller-interface.h"
#include "../../Interfaces/mysql-interface.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <confd_lib.h>
#include <confd_dp.h>
#include "hst.h"

#define bool short
#define true 1
#define false 0

#define DENY 0
#define PERMIT 1
#define MIRROR 2
#define ADVANCED 3

/********************************************************************/

struct policy {
    char rule_name[50];        // ID
    int rule_id;
    struct in_addr src_list[50];
    int src_num;
    struct in_addr dest_list[50];
    int dest_num;
    int start_time;
    int end_time;
    int action;
};

/********************************************************************/

/* Our daemon context as a global variable */
static struct confd_daemon_ctx *dctx;
static struct confd_trans_cbs trans;
static struct confd_data_cbs policy_cbks;

/* My user data, we got to install opaque data into */
/* the confd_daemon_ctx, this data is then accesible from the */
/* trans callbacks and must thus not necessarily vae to  */
/* be global data. */

struct mydata {
    int ctlsock;
    int workersock;
    int locked;
};

/* Help function which allocates a new host struct */
static struct policy *new_policy(char *name)
{
    struct policy *pp;
    if ((pp = (struct policy*) calloc(1, sizeof(struct policy))) == NULL)
        return NULL;
    strcpy(pp->rule_name, name);
    return pp;
}

/* Help function which insert policy to mysql */
static bool add_policy(struct policy *policy_container) {
    char columns[100] = {0}, values[200] = {0};
    int n = 0, i = 0, j = 0;

    strcpy(columns, "`policy-id`, `policy-name`");
    n = sprintf(values, "%d, \"%s\"", policy_container->rule_id, policy_container->rule_name);
    values[n] = '\0';

    if(MysqlInsertQuery("`firewall-policy`", columns, values)) {
        printf("\n\n New Policy Created \n\n");
        strcpy(columns, "`saddr`, `daddr`, `stime`, `etime`, `action`, `policy-id`");

        for(i = 0; i < policy_container->src_num; i++) {
            for(j = 0; j < policy_container->dest_num; j++) {
                n = sprintf(values, "%lu, %lu, %d, %d, %d, %d", 
                                   (unsigned long)(policy_container->src_list[i].s_addr),
                                   (unsigned long)(policy_container->dest_list[j].s_addr),
                                   policy_container->start_time,
                                   policy_container->end_time,
                                   policy_container->action,
                                   policy_container->rule_id);
                values[n] = '\0';
                if(!MysqlInsertQuery("`firewall-rule`", columns, values)) {
                    char where[20];
                    n = sprintf(where, "`policy-id`=%d", policy_container->rule_id);
                    where[n] = '\0';
                    MysqlDeleteQuery("`firewall-rule`", where);

                    fprintf(stderr, "mysql insert failed\n");
                    return false;        
                }
                printf("\n\n New Rule for The Policy Added \n\n");
            }
        }
    } else {
        fprintf(stderr, "mysql insert failed\n");
        return false;
    }

    return true;
}

static bool is_policy_exists(char *policy_name) {
    char where[100];
    MYSQL_RES *sqlResult;
    bool res = false;

    int n = sprintf(where, "`policy-name`=\"%s\"", policy_name);
    where[n] = '\0';

    sqlResult = MysqlSelectQuery("`firewall-policy`", "`policy-name`", where, true);
    if(MysqlGetNumRows(sqlResult) > 0) res = true;
    mysql_free_result(sqlResult);

    return res;
}


/********************************************************************/
/* transaction callbacks  */

/* The installed init() function gets called everytime Confd */
/* wants to establish a new transaction, Each NETCONF */
/* command will be a transaction */

/* We can choose to create threads here or whatever, we */
/* can choose to allocate this transaction to an already existing */
/* thread. We must tell Confd which filedescriptor should be */
/* used for all future communication in this transaction */
/* this has to be done through the call confd_trans_set_fd(); */

static int tr_init(struct confd_trans_ctx *tctx)
{
    char buf[INET6_ADDRSTRLEN];
    inet_ntop(tctx->uinfo->af, &tctx->uinfo->ip, buf, sizeof(buf));
    printf ("s_init() for %s from %s ", tctx->uinfo->username, buf);
    struct mydata *md = (struct mydata*) tctx->dx->d_opaque;
    confd_trans_set_fd(tctx, md->workersock);
    return CONFD_OK;
}

/* This callback gets invoked at the end of the transaction */
/* when ConfD has accumulated all write operations */
/* we're guaranteed that */
/* a) no more read ops will occur */
/* b) no other transactions will run between here and tr_finish() */
/*    for this transaction, i.e ConfD will serialize all transactions */

/* since we need to be prepared for abort(), we may not write */
/* our data to the actual database, we can choose to either */
/* copy the entire database here and write to the copy in the */
/* following write operatons _or_ let the write operations */
/* accumulate operations create(), set(), delete() instead of actually */
/* writing */

/* If our db supports transactions (which it doesn't in this */
/* silly example, this is the place to do START TRANSACTION */

static int tr_writestart(struct confd_trans_ctx *tctx)
{
    return CONFD_OK;
}

static int tr_prepare(struct confd_trans_ctx *tctx)
{
    return CONFD_OK;
}


static int tr_commit(struct confd_trans_ctx *tctx) // use the xml files.
{
    struct confd_tr_item *item = tctx->accumulated; // item is dats about xml files.
    struct policy *policy_container;
    confd_value_t *src_list, *dest_list;
    char where[100] = {0};
    int i;

    while (item) {
        confd_hkeypath_t *keypath = item->hkp;
        confd_value_t *leaf = &(keypath->v[0][0]);
        if (strcmp(item->callpoint, "hcp") == 0) {
            switch (item->op) {
            case C_SET_ELEM:
                switch(CONFD_GET_XMLTAG(leaf)) {
                case nsc_rule_id:
                    policy_container->rule_id = (int) CONFD_GET_UINT32(item->val);
                    break;
                case nsc_pkt_sec_cond_ipv4_src_addr:
                    src_list = CONFD_GET_LIST(item->val);
                    policy_container->src_num = CONFD_GET_LISTSIZE(item->val);

                    for(i = 0; i < policy_container->src_num; i++) {
                        policy_container->src_list[i] = CONFD_GET_IPV4(&src_list[i]);
                    }
                    break;
                case nsc_pkt_sec_cond_ipv4_dest_addr:
                    dest_list = CONFD_GET_LIST(item->val);
                    policy_container->dest_num = CONFD_GET_LISTSIZE(item->val);

                    for(i = 0; i < policy_container->dest_num; i++) {
                        policy_container->dest_list[i] = CONFD_GET_IPV4(&dest_list[i]);
                    }
 
                    break;
                case nsc_start_time:
                    policy_container->start_time = atoi((char *) CONFD_GET_BUFPTR(item->val)); // (int) CONFD_GET_UINT32(item->val);
                    break;
                case nsc_end_time:
                    policy_container->end_time = atoi((char *) CONFD_GET_BUFPTR(item->val)); //(int) CONFD_GET_UINT32(item->val);
                    break;
                case nsc_permit:
                    if(CONFD_GET_BOOL(item->val))
                        policy_container->action = PERMIT;
                    break;
                case nsc_deny:
                    if(CONFD_GET_BOOL(item->val))
                        policy_container->action = DENY;
                }
                break;
            case C_CREATE:
                // Create container
                policy_container = new_policy((char *)CONFD_GET_BUFPTR(leaf));
                
                break;
            case C_REMOVE:
                // Find policy and remove
                strcpy(where, "`policy-name`=");
                strcpy(where + 12, (char *)CONFD_GET_BUFPTR(leaf));
                if(!MysqlDeleteQuery("`firewall-policy`", where)) {
                    fprintf(stderr, "policy remove failed\n");
                    return CONFD_ERR;
                }
                break;
            default:
                return CONFD_ERR;
            }
        }
        item = item->next;
    }

    if(!is_policy_exists(policy_container->rule_name))
        add_policy(policy_container);
    else {
        fprintf(stderr, "\nsame policy name exists..\n");
        return CONFD_ERR;
    }

    return CONFD_OK;
}

static int tr_abort(struct confd_trans_ctx *tctx)
{
    return CONFD_OK;
}

static int tr_finish(struct confd_trans_ctx *tctx)
{
    return CONFD_OK;
}

static int policy_set_elem(struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath, confd_value_t *newval) {
    return CONFD_ACCUMULATE;
}

/* Data Exists Check */
static int policy_get_elem (struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath) {
/*
    int n = 0;
    char where[100] = {0};
    MYSQL_RES *sqlResult;
    switch(CONFD_GET_XMLTAG(&(keypath->v[0][0]))){
        case nsc_rule_name:
        // Check whether same rule name exists or not
        n = sprintf(where, "`policy-name`=\"%s\"", (char *) &(keypath->v[1][0]);)
        where[n] = '\0';
        printf("%s\n", (&(keypath->v[0][0])));
        printf("%s\n", (&(keypath->v[1][0])));
        printf("%s\n", (&(keypath->v[2][0])));

        sqlResult = MysqlSelectQuery("`firewall-policy`", "`policy-name`", where, true);
        printf("query success\n", &(keypath->v[3][0]));

        if(MysqlGetNumRows(sqlResult) > 0) {
            confd_value_t v;
            MYSQL_ROW row = MysqlGetRow(sqlResult);
            CONFD_SET_STR(&v, row[0]);
            confd_data_reply_value(tctx, &v);
        }
        mysql_free_result(sqlResult);

        break;

        case nsc_rule_id:
        // Check same rule id exists
        strcpy(where, "`policy-id`=");
        strcpy(where + 12, (char *) &(keypath->v[1][0]));
        sqlResult = MysqlSelectQuery("`firewall-policy`", "`policy-id`", where, true);

        if(MysqlGetNumRows(sqlResult) > 0) {
            confd_value_t v;
            MYSQL_ROW row = MysqlGetRow(sqlResult);
            CONFD_SET_INT32(&v, (int) *(row[0]));
            confd_data_reply_value(tctx, &v);
        }
        mysql_free_result(sqlResult);

        break;

        case nsc_pkt_sec_cond_ipv4_src_addr:
        case nsc_pkt_sec_cond_ipv4_dest_addr:
        case nsc_start_time:
        case nsc_end_time:
        case nsc_permit:
        case nsc_deny:
        // These tags are not unique property.
        break;

        default:
            fprintf(stderr, "HERE %d\n", CONFD_GET_XMLTAG(&(keypath->v[0][0])));
            return CONFD_ERR;
    }
*/
    confd_data_reply_not_found(tctx);
    return CONFD_OK;
}

static int policy_delete(struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath) {
    return CONFD_ACCUMULATE;
}
static int policy_get_next(struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath, long next) {
    return CONFD_OK;
}

static int policy_num_instances(struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath) {
    //return num of policies
    MYSQL_RES *sqlResult = MysqlSelectQuery("`firewall-policy`", "COUNT(*)", "1=1", false);
    MYSQL_ROW row = MysqlGetRow(sqlResult);

    confd_value_t v;
    CONFD_SET_INT32(&v, *(row[0]));
    confd_data_reply_value(tctx, &v);

    mysql_free_result(sqlResult);
    return CONFD_OK;
}

static int policy_create(struct confd_trans_ctx *tctx, confd_hkeypath_t *keypath) {
    return CONFD_ACCUMULATE;
}


void start_confd() {
    int ctlsock;
    int workersock;
    struct sockaddr_in addr;
    struct mydata *md;
    int debuglevel = CONFD_TRACE;

    //MysqlInitialize();

    /* These are our transaction callbacks */
    trans.init = tr_init;
    trans.write_start = tr_writestart;
    trans.prepare = tr_prepare;
    trans.commit = tr_commit;
    trans.abort = tr_abort;
    trans.finish = tr_finish;


    policy_cbks.get_elem = policy_get_elem;
    policy_cbks.get_next = policy_get_next;
    policy_cbks.num_instances = policy_num_instances;
    policy_cbks.set_elem = policy_set_elem;
    policy_cbks.create = policy_create;
    policy_cbks.remove = policy_delete;
    strcpy(policy_cbks.callpoint, "hcp");

    /* Init library  */
    confd_init("firewall_daemon", stderr, debuglevel);
    /* Initialize daemon context */
    if ((dctx = confd_init_daemon("firewall_daemon")) == NULL)
        confd_fatal("Failed to initialize confd\n");

    if ((ctlsock = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
        confd_fatal("Failed to open ctlsocket\n");

    
    //addr.sin_addr.s_addr = inet_addr("10.0.0.200");    //IMTL
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_family = AF_INET;
    addr.sin_port = htons(CONFD_PORT);

    if (confd_load_schemas((struct sockaddr*)&addr,
                           sizeof (struct sockaddr_in)) != CONFD_OK)
        confd_fatal("Failed to load schemas from confd\n");

    /* Create the first control socket, all requests to */
    /* create new transactions arrive here */
    if (confd_connect(dctx, ctlsock, CONTROL_SOCKET, (struct sockaddr*)&addr,
                      sizeof (struct sockaddr_in)) < 0)
        confd_fatal("Failed to confd_connect() to confd \n");


    /* Also establish a workersocket, this is the most simple */
    /* case where we have just one ctlsock and one workersock */
    if ((workersock = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
        confd_fatal("Failed to open workersocket\n");
    if (confd_connect(dctx, workersock, WORKER_SOCKET,(struct sockaddr*)&addr,
                      sizeof (struct sockaddr_in)) < 0)
        confd_fatal("Failed to confd_connect() to confd \n");


    /* Create a user datastructure and connect it to the */
    /* daemon struct so that we can always get to it */
    if ((md = dctx->d_opaque = (struct mydata*)
         calloc(1, sizeof(struct mydata))) == NULL)
        confd_fatal("Failed to malloc");
    md->ctlsock = ctlsock;
    md->workersock = workersock;

    confd_register_trans_cb(dctx, &trans);

    if (confd_register_data_cb(dctx, &policy_cbks) == CONFD_ERR)
        confd_fatal("Failed to register host cb \n");
    if (confd_register_done(dctx) != CONFD_OK)
        confd_fatal("Failed to complete registration \n");

    struct pollfd set[2];
    int ret;

    set[0].fd = ctlsock;
    set[0].events = POLLIN;
    set[0].revents = 0;

    set[1].fd = workersock;
    set[1].events = POLLIN;
    set[1].revents = 0;

    while (1) {
        if (poll(&set[0], 2, -1) < 0) {
            perror("Poll failed:");
            continue;
        }

        /* Check for I/O */
        if (set[0].revents & POLLIN) {
            if ((ret = confd_fd_ready(dctx, ctlsock)) == CONFD_EOF) {
                confd_fatal("Control socket closed\n");
            } else if (ret == CONFD_ERR && confd_errno != CONFD_ERR_EXTERNAL) {
                confd_fatal("Error on control socket request: %s (%d): %s\n",
                     confd_strerror(confd_errno), confd_errno, confd_lasterr());
            }
        }
        if (set[1].revents & POLLIN) {
            if ((ret = confd_fd_ready(dctx, workersock)) == CONFD_EOF) {
                confd_fatal("Worker socket closed\n");
            } else if (ret == CONFD_ERR && confd_errno != CONFD_ERR_EXTERNAL) {
                confd_fatal("Error on worker socket request: %s (%d): %s\n",
                     confd_strerror(confd_errno), confd_errno, confd_lasterr());
            }
        }
    }
}

/********************************************************************/
