#include "TinySync.h"
#include <map>
#include <memory>
#include <xhash>

struct SyncronizationInfo {
    void* DataPointer;
    size_t PreviousDataHash;
    size_t DataSize;
    int Operation;
};

struct SendingData {
    size_t StructSize;
    size_t DataNameHash;
    char DataBuffer[1];
};

static std::map<size_t, SyncronizationInfo> Variables;
static DataSendingFunction SendingFunction = nullptr;
static DataReceivingFunction ReceivingFunction = nullptr;

static size_t GetDataHash(void* Data, size_t DataSize) {
    return std::_Hash_bytes(static_cast<const unsigned char*>(Data), DataSize);
}

bool AddSyncronizedObject(const char DataName[], void* Data, size_t DataSize, int Operation) {
    Variables[std::hash_value(DataName)] = { Data, GetDataHash(Data, DataSize), DataSize, Operation };
	return false;
}

bool DeleteSyncronizedObject(const char DataName[]) {
    Variables.erase(std::hash_value(DataName));
	return false;
}

bool RegisterDataSendingFunction(DataSendingFunction Function) {
    SendingFunction = Function;
	return false;
}

bool RegisterDataReceivingFunction(DataReceivingFunction Function) {
    ReceivingFunction = Function;
	return false;
}

void ExecuteAllDataSyncronization() {
    for (auto& Item : Variables) {
        bool ExecuteSend = false;

        DataSyncronizationOperation SyncronizeTiming = static_cast<DataSyncronizationOperation>(Item.second.Operation & DataSyncronizationOperation::SyncronizeTimingOperationMask);
        bool SyncronizeWrite = Item.second.Operation & DataSyncronizationOperation::SyncronizeWrite;

        if (SyncronizeTiming == DataSyncronizationOperation::AlwaysSyncronize) {
            ExecuteSend = SyncronizeWrite;
        } else if (SyncronizeTiming == DataSyncronizationOperation::SyncronizeOnDataChanged) {
            size_t DataHash = GetDataHash(Item.second.DataPointer, Item.second.DataSize);

            if (DataHash != Item.second.PreviousDataHash) {
                ExecuteSend = SyncronizeWrite;
                Item.second.PreviousDataHash = DataHash;
            }
        }

        if (ExecuteSend) {
            auto AllocateSize = Item.second.DataSize + sizeof(size_t) * 2;
            auto SendData = static_cast<SendingData*>(_malloca(Item.second.DataSize + sizeof(size_t) * 2));
            SendData->StructSize = AllocateSize;
            SendData->DataNameHash = Item.first;
            memcpy_s(SendData->DataBuffer, AllocateSize - sizeof(size_t) * 2, Item.second.DataPointer, Item.second.DataSize);

            SendingFunction(SendData, AllocateSize);

            _freea(SendData);
        }
    }

    size_t StructSize;

    while (ReceivingFunction(&StructSize, sizeof(size_t)) == sizeof(size_t)) {
        auto ReceivingData = static_cast<SendingData*>(_malloca(StructSize));

        if (ReceivingFunction(&ReceivingData->DataNameHash, StructSize - sizeof(size_t)) == StructSize - sizeof(size_t)) {
            bool ExecuteReceive = false;
            auto& Item = Variables[ReceivingData->DataNameHash];

            DataSyncronizationOperation SyncronizeTiming = static_cast<DataSyncronizationOperation>(Item.Operation & DataSyncronizationOperation::SyncronizeTimingOperationMask);
            bool SyncronizeRead = Item.Operation & DataSyncronizationOperation::SyncronizeRead;

            if (SyncronizeTiming != DataSyncronizationOperation::SyncronizeExplicitly) {
                ExecuteReceive = SyncronizeRead;
            }

            if (ExecuteReceive) {
                memcpy_s(Item.DataPointer, Item.DataSize, ReceivingData->DataBuffer, StructSize - sizeof(size_t) * 2);
                Item.PreviousDataHash = GetDataHash(Item.DataPointer, Item.DataSize);
            }
        }
    }
}

void ExecuteDataSyncronization(const char DataName[]) {
    auto VariableHash = std::hash_value(DataName);
    auto& Item = Variables[VariableHash];
 
    auto AllocateSize = Item.DataSize + sizeof(size_t) * 2;
    auto SendData = static_cast<SendingData*>(_malloca(Item.DataSize + sizeof(size_t) * 2));
    SendData->StructSize = AllocateSize;
    SendData->DataNameHash = VariableHash;
    memcpy_s(SendData->DataBuffer, AllocateSize - sizeof(size_t) * 2, Item.DataPointer, Item.DataSize);

    Item.PreviousDataHash = GetDataHash(Item.DataPointer, Item.DataSize);
    SendingFunction(SendData, AllocateSize);

    _freea(SendData);
}