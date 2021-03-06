#ifndef _METADATA_RESPONSE_H_
#define _METADATA_RESPONSE_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "response.h"
#include "broker.h"
#include "partition.h"

class PartitionMetadata {
public:
	PartitionMetadata(char **buf);

	int CountSize();
	void PrintAll();

	friend class MetadataResponse;
private:
	int16_t partition_error_code_;
	int32_t partition_id_;
	int32_t leader_;
	std::vector<int> replicas_;		// array
	std::vector<int> isr_;			// array
};

class TopicMetadata {
public:
	TopicMetadata(char **buf);

	int CountSize();
	void PrintAll();

	friend class MetadataResponse;
private:
	int16_t topic_error_code_;
	std::string topic_;
	std::vector<std::shared_ptr<PartitionMetadata>> partition_metadata_;
};

// format: [Broker][TopicMetadata]
class MetadataResponse: public Response {
public:
	MetadataResponse(char **buf);
	virtual ~MetadataResponse() {}

	virtual int CountSize();
	virtual void PrintAll();

	void ParseBrokers(std::unordered_map<int, Broker> &updated_brokers);
	int16_t ParsePartitions(std::unordered_map<int, Partition> &partitions);

private:
	std::vector<std::shared_ptr<Broker>> brokers_;				// array
	std::vector<std::shared_ptr<TopicMetadata>> topic_metadata_;	// array
	int GetFdFromIp(const std::string &alive_ip, const std::unordered_map<int, Broker> &origin_brokers);
};

#endif
