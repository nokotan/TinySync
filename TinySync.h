#pragma once

typedef size_t (*DataSendingFunction)(void*, size_t);
typedef size_t (*DataReceivingFunction)(void*, size_t);

enum DataSyncronizationOperation {
    AlwaysSyncronize = 0,
    SyncronizeOnDataChanged,
    SyncronizeExplicitly,
    SyncronizeTimingOperationMask = 3,
    SyncronizeWrite = 32,
    SyncronizeRead = 64,
    SyncronizeReadWrite = SyncronizeWrite | SyncronizeRead
};

bool AddSyncronizedObject(const char DataName[], void *Data, size_t DataSize, int = DataSyncronizationOperation::SyncronizeOnDataChanged);

bool DeleteSyncronizedObject(const char DataName[]);

bool RegisterDataSendingFunction(DataSendingFunction Function);

bool RegisterDataReceivingFunction(DataReceivingFunction Function);

void ExecuteAllDataSyncronization();

void ExecuteDataSyncronization(const char DataName[]);