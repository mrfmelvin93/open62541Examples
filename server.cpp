/* This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information. */

#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Build Instructions (Linux)
 * - gcc server.c -lopen62541 -o server */

// Maximum number of variables to add
#define MAX_VARIABLES 10000

// Struct to hold information about the variable
typedef struct {
    UA_NodeId nodeId;
    const UA_DataType *dataType;
} VariableInfo;

// Array to store variable information
VariableInfo variables[MAX_VARIABLES];

// Function to add a folder to the server
UA_NodeId addFolder(UA_Server *server, char *folderName, UA_NodeId parentNodeId, int namespaceIndex) {
    UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
    oAttr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", folderName);
    UA_NodeId folderNodeId = UA_NODEID_NULL;

    UA_Server_addObjectNode(server,
                            UA_NODEID_NULL,
                            parentNodeId,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                            UA_QUALIFIEDNAME(namespaceIndex, folderName),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE),
                            oAttr, NULL, &folderNodeId);

    return folderNodeId;
}

// Function to add a variable to the server
void addVariable(UA_Server *server, UA_NodeId parentFolderNodeId, const char *variableName, const UA_Variant *variantValue, const UA_DataType *dataType, int index) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    UA_Variant_setScalarCopy(&attr.value, variantValue, dataType);
    attr.description = UA_LOCALIZEDTEXT_ALLOC("en-US", variableName);
    attr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", variableName);
    
    UA_NodeId nodeId = UA_NODEID_STRING_ALLOC(parentFolderNodeId.namespaceIndex, variableName);
    UA_QualifiedName nodeName = UA_QUALIFIEDNAME_ALLOC(parentFolderNodeId.namespaceIndex, variableName);
    
    UA_Server_addVariableNode(server, nodeId, parentFolderNodeId,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), nodeName,
                              UA_NODEID_NULL, attr, NULL, NULL);
                              
    // Store variable information
    variables[index].nodeId = nodeId;
    variables[index].dataType = dataType;

    UA_VariableAttributes_clear(&attr);
    UA_QualifiedName_clear(&nodeName);
}

// Callback function to update variable values
static void updateVariablesCallback(UA_Server *server, void *data) {
    for (int i = 0; i < MAX_VARIABLES; i++) {
        if(variables[i].dataType == NULL) continue;

        UA_Variant value;
        UA_Variant_init(&value);

        // Update the variable value based on its type
        if (variables[i].dataType == &UA_TYPES[UA_TYPES_INT32]) {
            static UA_Int32 intValue = 0;
            intValue++;
            UA_Variant_setScalar(&value, &intValue, &UA_TYPES[UA_TYPES_INT32]);
        } else if (variables[i].dataType == &UA_TYPES[UA_TYPES_FLOAT]) {
            static UA_Float floatValue = 0.0f;
            floatValue += 1.0f;
            UA_Variant_setScalar(&value, &floatValue, &UA_TYPES[UA_TYPES_FLOAT]);
        } else if (variables[i].dataType == &UA_TYPES[UA_TYPES_DOUBLE]) {
            static UA_Double doubleValue = 0.0;
            doubleValue += 1.0;
            UA_Variant_setScalar(&value, &doubleValue, &UA_TYPES[UA_TYPES_DOUBLE]);
        } else if (variables[i].dataType == &UA_TYPES[UA_TYPES_BOOLEAN]) {
            static UA_Boolean booleanValue = UA_TRUE;
            booleanValue = !booleanValue;
            UA_Variant_setScalar(&value, &booleanValue, &UA_TYPES[UA_TYPES_BOOLEAN]);
        } else if (variables[i].dataType == &UA_TYPES[UA_TYPES_STRING]) {
            static int stringCounter = 0;
            char buffer[50];
            snprintf(buffer, sizeof(buffer), "Dynamic String %d", stringCounter++);
            UA_String strValue = UA_STRING_ALLOC(buffer);
            UA_Variant_setScalar(&value, &strValue, &UA_TYPES[UA_TYPES_STRING]);
            UA_String_clear(&strValue);
        } else if (variables[i].dataType == &UA_TYPES[UA_TYPES_DATETIME]) {
            UA_DateTime dateTimeValue = UA_DateTime_now();
            UA_Variant_setScalar(&value, &dateTimeValue, &UA_TYPES[UA_TYPES_DATETIME]);
        } else if (variables[i].dataType == &UA_TYPES[UA_TYPES_BYTE]) {
            static UA_Byte byteValue = 0;
            byteValue++;
            UA_Variant_setScalar(&value, &byteValue, &UA_TYPES[UA_TYPES_BYTE]);
        } else if (variables[i].dataType == &UA_TYPES[UA_TYPES_UINT16]) {
            static UA_UInt16 uint16Value = 0;
            uint16Value++;
            UA_Variant_setScalar(&value, &uint16Value, &UA_TYPES[UA_TYPES_UINT16]);
        } else if (variables[i].dataType == &UA_TYPES[UA_TYPES_UINT32]) {
            static UA_UInt32 uint32Value = 0;
            uint32Value++;
            UA_Variant_setScalar(&value, &uint32Value, &UA_TYPES[UA_TYPES_UINT32]);
        } else if (variables[i].dataType == &UA_TYPES[UA_TYPES_INT16]) {
            static UA_Int16 int16Value = 0;
            int16Value++;
            UA_Variant_setScalar(&value, &int16Value, &UA_TYPES[UA_TYPES_INT16]);
        } else if (variables[i].dataType == &UA_TYPES[UA_TYPES_INT64]) {
            static UA_Int64 int64Value = 0;
            int64Value++;
            UA_Variant_setScalar(&value, &int64Value, &UA_TYPES[UA_TYPES_INT64]);
        } else if (variables[i].dataType == &UA_TYPES[UA_TYPES_UINT64]) {
            static UA_UInt64 uint64Value = 0;
            uint64Value++;
            UA_Variant_setScalar(&value, &uint64Value, &UA_TYPES[UA_TYPES_UINT64]);
        } else if (variables[i].dataType == &UA_TYPES[UA_TYPES_SBYTE]) {
            static UA_SByte sbyteValue = 0;
            sbyteValue++;
            UA_Variant_setScalar(&value, &sbyteValue, &UA_TYPES[UA_TYPES_SBYTE]);
        }

        // Write the updated value to the variable node
        UA_Server_writeValue(server, variables[i].nodeId, value);
    }
}

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: %s <number_of_variables>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int numVariables = atoi(argv[1]);
    if(numVariables > MAX_VARIABLES) {
        printf("The maximum number of variables is %d.\n", MAX_VARIABLES);
        return EXIT_FAILURE;
    }

    UA_Server *server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));

    int namespaceIndex = UA_Server_addNamespace(server, "CustomNamespace");

    // Create folders for each datatype
    UA_NodeId intFolder = addFolder(server, "IntegerFolder", UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), namespaceIndex);
    UA_NodeId floatFolder = addFolder(server, "FloatFolder", UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), namespaceIndex);
    UA_NodeId doubleFolder = addFolder(server, "DoubleFolder", UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), namespaceIndex);
    UA_NodeId boolFolder = addFolder(server, "BooleanFolder", UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), namespaceIndex);
    UA_NodeId stringFolder = addFolder(server, "StringFolder", UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), namespaceIndex);
    UA_NodeId datetimeFolder = addFolder(server, "DateTimeFolder", UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), namespaceIndex);
    UA_NodeId byteFolder = addFolder(server, "ByteFolder", UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), namespaceIndex);
    UA_NodeId uint16Folder = addFolder(server, "UInt16Folder", UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), namespaceIndex);
    UA_NodeId uint32Folder = addFolder(server, "UInt32Folder", UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), namespaceIndex);
    UA_NodeId int16Folder = addFolder(server, "Int16Folder", UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), namespaceIndex);
    UA_NodeId int64Folder = addFolder(server, "Int64Folder", UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), namespaceIndex);
    UA_NodeId uint64Folder = addFolder(server, "UInt64Folder", UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), namespaceIndex);
    UA_NodeId sbyteFolder = addFolder(server, "SByteFolder", UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), namespaceIndex);

    // Variables to add
    char* dataTypeNames[] = {
        "Integer", "Float", "Double", "Boolean", "String", "DateTime", "Byte", 
        "UInt16", "UInt32", "Int16", "Int64", "UInt64", "SByte"
    };
    const UA_DataType* dataTypes[] = {
        &UA_TYPES[UA_TYPES_INT32], &UA_TYPES[UA_TYPES_FLOAT], &UA_TYPES[UA_TYPES_DOUBLE], 
        &UA_TYPES[UA_TYPES_BOOLEAN], &UA_TYPES[UA_TYPES_STRING], &UA_TYPES[UA_TYPES_DATETIME], 
        &UA_TYPES[UA_TYPES_BYTE], &UA_TYPES[UA_TYPES_UINT16], &UA_TYPES[UA_TYPES_UINT32], 
        &UA_TYPES[UA_TYPES_INT16], &UA_TYPES[UA_TYPES_INT64], &UA_TYPES[UA_TYPES_UINT64], 
        &UA_TYPES[UA_TYPES_SBYTE]
    };

    UA_NodeId parentFolders[] = {
        intFolder, floatFolder, doubleFolder, boolFolder, stringFolder, datetimeFolder,
        byteFolder, uint16Folder, uint32Folder, int16Folder, int64Folder, uint64Folder, sbyteFolder
    };

    int numDataTypes = sizeof(dataTypes) / sizeof(dataTypes[0]);
    int counter = 0;

    // Loop to create variables
    for (int i = 0; i < numVariables / numDataTypes; i++) {
        for (int j = 0; j < numDataTypes; j++) {
            // Generate unique name for each variable
            char variableName[50];
            snprintf(variableName, sizeof(variableName), "%s_%d", dataTypeNames[j], counter);
            counter++;

            // Create variant value
            UA_Variant variant;
            UA_Variant_init(&variant);
            
            if (strcmp(dataTypeNames[j], "Integer") == 0) {
                UA_Int32 value = 0;
                UA_Variant_setScalar(&variant, &value, dataTypes[j]);
            } else if (strcmp(dataTypeNames[j], "Float") == 0) {
                UA_Float value = 0.0f;
                UA_Variant_setScalar(&variant, &value, dataTypes[j]);
            } else if (strcmp(dataTypeNames[j], "Double") == 0) {
                UA_Double value = 0.0;
                UA_Variant_setScalar(&variant, &value, dataTypes[j]);
            } else if (strcmp(dataTypeNames[j], "Boolean") == 0) {
                UA_Boolean value = UA_FALSE;
                UA_Variant_setScalar(&variant, &value, dataTypes[j]);
            } else if (strcmp(dataTypeNames[j], "String") == 0) {
                UA_String value = UA_STRING_ALLOC("Initial String");
                UA_Variant_setScalar(&variant, &value, dataTypes[j]);
                UA_String_clear(&value);
            } else if (strcmp(dataTypeNames[j], "DateTime") == 0) {
                UA_DateTime value = UA_DateTime_now();
                UA_Variant_setScalar(&variant, &value, dataTypes[j]);
            } else if (strcmp(dataTypeNames[j], "Byte") == 0) {
                UA_Byte value = 0;
                UA_Variant_setScalar(&variant, &value, dataTypes[j]);
            } else if (strcmp(dataTypeNames[j], "UInt16") == 0) {
                UA_UInt16 value = 0;
                UA_Variant_setScalar(&variant, &value, dataTypes[j]);
            } else if (strcmp(dataTypeNames[j], "UInt32") == 0) {
                UA_UInt32 value = 0;
                UA_Variant_setScalar(&variant, &value, dataTypes[j]);
            } else if (strcmp(dataTypeNames[j], "Int16") == 0) {
                UA_Int16 value = 0;
                UA_Variant_setScalar(&variant, &value, dataTypes[j]);
            } else if (strcmp(dataTypeNames[j], "Int64") == 0) {
                UA_Int64 value = 0;
                UA_Variant_setScalar(&variant, &value, dataTypes[j]);
            } else if (strcmp(dataTypeNames[j], "UInt64") == 0) {
                UA_UInt64 value = 0;
                UA_Variant_setScalar(&variant, &value, dataTypes[j]);
            } else if (strcmp(dataTypeNames[j], "SByte") == 0) {
                UA_SByte value = 0;
                UA_Variant_setScalar(&variant, &value, dataTypes[j]);
            }

            addVariable(server, parentFolders[j], variableName, &variant, dataTypes[j], counter - 1);
        }
    }

    // Add repeated callback to update variable values every second
    UA_Server_addRepeatedCallback(server, updateVariablesCallback, NULL, 1000, NULL);

    UA_StatusCode retval = UA_Server_runUntilInterrupt(server);
    UA_Server_delete(server);
    
    return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}
