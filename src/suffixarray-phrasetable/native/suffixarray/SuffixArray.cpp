//
// Created by Davide  Caroselli on 28/09/16.
//

#include "SuffixArray.h"
#include "dbkv.h"
#include <rocksdb/slice_transform.h>
#include <rocksdb/merge_operator.h>
#include <boost/filesystem.hpp>
#include <thread>

namespace fs = boost::filesystem;

using namespace rocksdb;
using namespace mmt;
using namespace mmt::sapt;

static const string kGlobalInfoKey = MakeEmptyKey(kGlobalInfoKeyType);

/*
 * MergePositionOperator
 */

namespace mmt {
    namespace sapt {

        class MergePositionOperator : public AssociativeMergeOperator {
        public:
            virtual bool Merge(const Slice &key, const Slice *existing_value, const Slice &value, string *new_value,
                               Logger *logger) const override {
                switch (key.data_[0]) {
                    case kSourcePrefixKeyType:
                    case kTargetPrefixKeyType:
                        MergePositionLists(existing_value, value, new_value);
                        return true;
                    default:
                        return false;
                }
            }

            inline void MergePositionLists(const Slice *existing_value, const Slice &value, string *new_value) const {
                if (existing_value)
                    *new_value = existing_value->ToString() + value.ToString();
                else
                    *new_value = value.ToString();
            }

            virtual const char *Name() const override {
                return "MergePositionOperator";
            }
        };

    }
}

/*
 * SuffixArray - Initialization
 */

SuffixArray::SuffixArray(const string &modelPath, uint8_t prefixLength,
                         bool prepareForBulkLoad) throw(index_exception, storage_exception) :
        prefixLength(prefixLength) {
    fs::path modelDir(modelPath);

    if (!fs::is_directory(modelDir))
        throw invalid_argument("Invalid model path: " + modelPath);

    fs::path storageFile = fs::absolute(modelDir / fs::path("corpora.bin"));
    fs::path indexPath = fs::absolute(modelDir / fs::path("index"));

    rocksdb::Options options;
    options.create_if_missing = true;
    options.merge_operator.reset(new MergePositionOperator);
    options.max_open_files = -1;
    options.compaction_style = kCompactionStyleLevel;

    if (prepareForBulkLoad) {
        options.PrepareForBulkLoad();
    } else {
        unsigned cpus = thread::hardware_concurrency();

        if (cpus > 1)
            options.IncreaseParallelism(cpus > 4 ? 4 : 2);

        options.level0_file_num_compaction_trigger = 8;
        options.level0_slowdown_writes_trigger = 17;
        options.level0_stop_writes_trigger = 24;
        options.num_levels = 4;

        options.write_buffer_size = 64L * 1024L * 1024L;
        options.max_write_buffer_number = 3;
        options.target_file_size_base = 64L * 1024L * 1024L;
        options.max_bytes_for_level_base = 512L * 1024L * 1024L;
        options.max_bytes_for_level_multiplier = 8;
    }

    Status status = DB::Open(options, indexPath.string(), &db);
    if (!status.ok())
        throw index_exception(status.ToString());

    db->CompactRange(CompactRangeOptions(), NULL, NULL);

    // Read streams
    string raw_streams;
    int64_t storageSize = 0;

    db->Get(ReadOptions(), kGlobalInfoKey, &raw_streams);
    DeserializeGlobalInfo(raw_streams.data(), raw_streams.size(), &storageSize, &streams);

    // Load storage
    storage = new CorpusStorage(storageFile.string(), storageSize);
}

SuffixArray::~SuffixArray() {
    delete db;
    delete storage;
}

/*
 * SuffixArray - Indexing
 */

void SuffixArray::ForceCompaction() {
    db->CompactRange(CompactRangeOptions(), NULL, NULL);
}

void SuffixArray::PutBatch(UpdateBatch &batch) throw(index_exception, storage_exception) {
    WriteBatch writeBatch;

    // Compute prefixes
    unordered_map<string, PostingList> sourcePrefixes;
    unordered_map<string, PostingList> targetPrefixes;

    for (auto entry = batch.data.begin(); entry != batch.data.end(); ++entry) {
        domain_t domain = entry->domain;

        int64_t offset = storage->Append(entry->source, entry->target, entry->alignment);
        AddPrefixesToBatch(true, domain, entry->source, offset, sourcePrefixes);
        AddPrefixesToBatch(false, domain, entry->target, offset, targetPrefixes);
    }

    int64_t storageSize = storage->Flush();

    // Add prefixes to write batch
    for (auto prefix = sourcePrefixes.begin(); prefix != sourcePrefixes.end(); ++prefix) {
        string value = prefix->second.Serialize();
        writeBatch.Merge(prefix->first, value);
    }
    for (auto prefix = targetPrefixes.begin(); prefix != targetPrefixes.end(); ++prefix) {
        string value = prefix->second.Serialize();
        writeBatch.Merge(prefix->first, value);
    }

    // Write global info
    writeBatch.Put(kGlobalInfoKey, SerializeGlobalInfo(batch.streams, storageSize));

    // Commit write batch
    Status status = db->Write(WriteOptions(), &writeBatch);
    if (!status.ok())
        throw index_exception("Unable to write to index: " + status.ToString());

    // Reset streams and domains
    streams = batch.GetStreams();
}

void SuffixArray::AddPrefixesToBatch(bool isSource, domain_t domain, const vector<wid_t> &sentence,
                                     int64_t location, unordered_map<string, PostingList> &outBatch) {
    size_t size = sentence.size();

    for (size_t start = 0; start < size; ++start) {
        for (size_t length = 1; length <= prefixLength; ++length) {
            if (start + length > size)
                break;

            string dkey = MakePrefixKey(prefixLength, isSource, domain, sentence, start, length);
            outBatch[dkey].Append(domain, location, (length_t) start);
        }
    }
}

/*
 * SuffixArray - Query
 */

size_t SuffixArray::CountOccurrences(bool isSource, const vector<wid_t> &phrase) {
    size_t count = 0;
    PrefixCursor *cursor = PrefixCursor::NewGlobalCursor(db, prefixLength, isSource);

    if (phrase.size() <= prefixLength) {
        for (cursor->Seek(phrase); cursor->HasNext(); cursor->Next())
            count += cursor->CountValue();
    } else {
        shared_ptr<PostingList> postingList;
        count = Collector::CollectLocations(cursor, phrase, prefixLength, 0, postingList);
    }

    delete cursor;
    return count;
}

void SuffixArray::GetRandomSamples(const vector<wid_t> &phrase, size_t limit, vector<sample_t> &outSamples,
                                   const context_t *context, bool searchInBackground) {
    Collector collector(storage, db, prefixLength, context, searchInBackground);
    collector.Extend(phrase, limit, outSamples);
}

Collector *SuffixArray::NewCollector(const context_t *context, bool searchInBackground) {
    return new Collector(storage, db, prefixLength, context, searchInBackground);
}



