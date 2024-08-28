#include <open62541/client_config_default.h>
#include <open62541/client_subscriptions.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/client_highlevel.h>

#define NAMESPACE_INDEX 0 // Replace with the correct namespace index for your server
#define FOLDER_NODEID 50842 // Replace with the correct NodeId of your folder

// Callback function to handle data change notifications
static void handler_TheAnswerChanged(UA_Client *client, UA_UInt32 subscriptionId, void *subscriptionContext,
                                     UA_UInt32 monitoredItemId, void *monitoredItemContext, UA_DataValue *value) {
    if (value->hasValue) {
        UA_Variant *val = &value->value;
        
        // Check the type of the variable and print accordingly
        if (UA_Variant_isScalar(val)) {
            if (val->type == &UA_TYPES[UA_TYPES_DOUBLE]) {
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                            "Subscription %u, Item %u: Double Value: %lf",
                            subscriptionId, monitoredItemId, *(UA_Double *)val->data);
            } else if (val->type == &UA_TYPES[UA_TYPES_INT16]) {
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                            "Subscription %u, Item %u: Int16 Value: %d",
                            subscriptionId, monitoredItemId, *(UA_Int32 *)val->data);
            } else if (val->type == &UA_TYPES[UA_TYPES_STRING]) {
                UA_String *str = (UA_String *)val->data;
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                            "Subscription %u, Item %u: String Value: %.*s",
                            subscriptionId, monitoredItemId, (int)str->length, str->data);
            } else if (val->type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                            "Subscription %u, Item %u: Boolean Value: %s",
                            subscriptionId, monitoredItemId, (*(UA_Boolean *)val->data) ? "true" : "false");
            } else {
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                            "Subscription %u, Item %u: Value of unknown type received",
                            subscriptionId, monitoredItemId);
            }
        } else {
            UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                        "Subscription %u, Item %u: Array or complex value received",
                        subscriptionId, monitoredItemId);
        }
    }
}

// Function to browse and subscribe to all variables in a specified folder
static void browseAndSubscribe(UA_Client *client, UA_NodeId folderNodeId) {
    UA_BrowseRequest bReq;
    UA_BrowseRequest_init(&bReq);
    bReq.requestedMaxReferencesPerNode = 0;
    bReq.nodesToBrowse = UA_BrowseDescription_new();
    bReq.nodesToBrowseSize = 1;
    bReq.nodesToBrowse[0].nodeId = folderNodeId;
    bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL;

    UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);
    
    UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
    UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(client, request, NULL, NULL, NULL);
    
    if (response.responseHeader.serviceResult != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Failed to create subscription.");
        return;
    }

    UA_UInt32 subId = response.subscriptionId;
    
    for (size_t i = 0; i < bResp.resultsSize; ++i) {
        for (size_t j = 0; j < bResp.results[i].referencesSize; ++j) {
            UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
            if (ref->nodeClass == UA_NODECLASS_VARIABLE) {
                UA_NodeId targetNodeId = UA_NODEID_STRING(ref->nodeId.nodeId.namespaceIndex,
                                                          ref->nodeId.nodeId.identifier.string.data);
                UA_MonitoredItemCreateRequest monRequest = UA_MonitoredItemCreateRequest_default(targetNodeId);

                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                            "Subscribing to Variable: %.*s", (int)ref->browseName.name.length, ref->browseName.name.data);

                UA_MonitoredItemCreateResult monResponse =
                    UA_Client_MonitoredItems_createDataChange(client, subId, UA_TIMESTAMPSTORETURN_BOTH,
                                                              monRequest, NULL, handler_TheAnswerChanged, NULL);
                
                if (monResponse.statusCode == UA_STATUSCODE_GOOD) {
                    printf("The subscription is successful, id %u\n", monResponse.monitoredItemId);
                } else {
                    printf("Subscription Failed\n");
                }
            }
        }
    }

    UA_BrowseRequest_clear(&bReq);
    UA_BrowseResponse_clear(&bResp);
}

int main(int argc, char *argv[]) {
    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));

    UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://localhost:4840"); // Change to your OPC UA server address
    if (retval != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Failed to connect to OPC UA server.");
        UA_Client_delete(client);
        return (int)retval;
    }

    UA_NodeId folderNodeId = UA_NODEID_NUMERIC(NAMESPACE_INDEX, FOLDER_NODEID);
    browseAndSubscribe(client, folderNodeId);

    while (1) {
        // Do nothing, just keep running
        printf("Client is running\n");
        UA_Client_run_iterate(client, 1000);
        sleep(1);
    }

    UA_Client_disconnect(client);
    UA_Client_delete(client);
    return 0;
}
