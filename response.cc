#include <iostream>

#include "response.h"
#include "request.h"
#include "util.h"

//------------------------------Head
Response::Response(short api_key, int correlation_id)
{
	api_key_ = api_key;
	total_size_ = 0;
	correlation_id_ = correlation_id;
}

Response::Response(short api_key, char **buf)
{
	api_key_ = api_key;
	total_size_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	correlation_id_ = Util::NetBytesToInt(*buf); 
	(*buf) += 4;
}

int Response::Size()
{
	return 4;
}

void Response::Print()
{
	std::cout << "api key = " << api_key_ << std::endl;
	std::cout << "total size = " << total_size_ << std::endl;
	std::cout << "correlation id = " << correlation_id_ << std::endl;
}

//------------------------------GroupCoordinatorResponse
GroupCoordinatorResponse::GroupCoordinatorResponse(int correlation_id, short error_code,
		int coordinator_id, const std::string &coordinator_host, int coordinator_port)
	: Response(ApiKey::GroupCoordinatorRequest, correlation_id)
{
	error_code_ = error_code;
	coordinator_id_ = coordinator_id;
	coordinator_host_ = coordinator_host;
	coordinator_port_ = coordinator_port;
	total_size_ = Size();
}

GroupCoordinatorResponse::GroupCoordinatorResponse(char **buf)
	: Response(ApiKey::GroupCoordinatorRequest, buf)
{
	error_code_ = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	coordinator_id_ = Util::NetBytesToInt(*buf); 
	(*buf) += 4;
	short host_size = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	coordinator_host_ = std::string(*buf, host_size);
	(*buf) += host_size;
	coordinator_port_ = Util::NetBytesToInt(*buf); 
}

int GroupCoordinatorResponse::Size()
{
	int size = 0;
	size = Response::Size() + 2 /* error_code */ + 4 /* coordinator_id */+
		   2 /* coordinator_host size */ + coordinator_host_.length() + 4;
	return size;
}

void GroupCoordinatorResponse::Print()
{
	std::cout << "-----GroupCoordinatorResponse-----" << std::endl;
	Response::Print();
	std::cout << "error code = " << error_code_ << std::endl;
	std::cout << "coordinator id = " << coordinator_id_ << std::endl;
	std::cout << "coordinator host = " << coordinator_host_ << std::endl;
	std::cout << "coordinator port = " << coordinator_port_ << std::endl;
	std::cout << "----------------------------------" << std::endl;
}

//------------------------------JoinGroupResponse
Member::Member(const std::string &member_id, const std::string &member_metadata)
{
	member_id_ = member_id;
	member_metadata_ = member_metadata;
}

int Member::Size()
{
	return 2 + member_id_.length() +
		   4 /* bytes */ + member_metadata_.length();
}

void Member::Print()
{
	std::cout << "member id = " << member_id_ << std::endl;
	std::cout << "member metadata = " << member_metadata_ << std::endl;
}

JoinGroupResponse::JoinGroupResponse(int correlation_id, short error_code,
		int generation_id, const std::string &group_protocol, const std::string &leader_id,
		const std::string &member_id, const std::vector<Member> &members)
	: Response(ApiKey::JoinGroupRequest, correlation_id)
{
	error_code_ = error_code;
	generation_id_ = generation_id;
	group_protocol_ = group_protocol;
	leader_id_ = leader_id;
	member_id_ = member_id;
	members_ = members;

	total_size_ = Size();
}

JoinGroupResponse::JoinGroupResponse(char **buf)
	: Response(ApiKey::JoinGroupRequest, buf)
{
	error_code_ = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	generation_id_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	// GroupProtocol
	short group_protocol_size = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	group_protocol_ = std::string(*buf, group_protocol_size);
	(*buf) += group_protocol_size;

	// LeaderId
	short leader_id_size = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	leader_id_ = std::string(*buf, leader_id_size);
	(*buf) += leader_id_size;

	// MemberId
	short member_id_size = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	member_id_ = std::string(*buf, member_id_size);
	(*buf) += member_id_size;

	// Members
	int members_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int i = 0; i < members_size; i++)
	{
		// MemberId
		short member_id_size = Util::NetBytesToShort(*buf);
		(*buf) += 2;
		std::string member_id(*buf, member_id_size);
		(*buf) += member_id_size;

		// MemberMetadata
		int member_metadata_size = Util::NetBytesToInt(*buf);
		(*buf) += 4;
		std::string member_metadata(*buf, member_metadata_size);
		(*buf) += member_metadata_size;

		Member member(member_id, member_metadata);
		members_.push_back(member);
	}
}

int JoinGroupResponse::Size()
{
	int size = 0;

	size = Response::Size() + 2 /* error_code */ + 4 /* generation_id */+
		   2 /* group_protocol size */ + group_protocol_.length() +
		   2 /* leader_id size */ + leader_id_.length() +
		   2 /* member_id size */ + member_id_.length();

	// array
	size += 4;
	for (unsigned int i = 0; i < members_.size(); i++)
		size += members_[i].Size();

	return size;
}

void JoinGroupResponse::Print()
{
	std::cout << "-----JoinGroupResponse-----" << std::endl;
	Response::Print();
	std::cout << "error code = " << error_code_ << std::endl;
	std::cout << "generation id = " << generation_id_ << std::endl;
	std::cout << "group protocol = " << group_protocol_ << std::endl;
	std::cout << "leader id = " << leader_id_ << std::endl;
	std::cout << "member id = " << member_id_ << std::endl;
	std::cout << "members:" << std::endl;
	for (auto it = members_.begin(); it != members_.end(); ++it)
	{
		Member &member = *it;
		member.Print();
	}
	std::cout << "---------------------------" << std::endl;
}

//------------------------------MetadataResponse
Broker::Broker(int node_id, const std::string &host, int port)
{
	node_id_ = node_id;
	host_ = host;
	port_ = port;
}

Broker::Broker(char **buf)
{
	// node id
	node_id_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	// host name
	short host_size = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	host_ = std::string(*buf, host_size);
	(*buf) += host_size;

	// port
	port_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;
}

int Broker::Size()
{
	return 4 + 2 + host_.length() + 4;
}

void Broker::Print()
{
	std::cout << "node id = " << node_id_ << std::endl;
	std::cout << "host = " << host_ << std::endl;
	std::cout << "port = " << port_ << std::endl;
}

PartitionMetadata::PartitionMetadata(short error_code, int partition_id, int leader,
		const std::vector<int> &replicas, const std::vector<int> &isr)
{
	partition_error_code_ = error_code;
	partition_id_ = partition_id;
	leader_ = leader;
	replicas_ = replicas;
	isr_ = isr;
}

PartitionMetadata::PartitionMetadata(char **buf)
{
	// partition error code
	partition_error_code_ = Util::NetBytesToShort(*buf);
	(*buf) += 2;

	// partition id
	partition_id_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	// leader
	leader_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	// replicas array size
	int replicas_array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int k = 0; k < replicas_array_size; k++)
	{
		int rep = Util::NetBytesToInt(*buf);
		(*buf) += 4;
		replicas_.push_back(rep);
	}

	// isr array size
	int isr_array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int k = 0; k < isr_array_size; k++)
	{
		isr_.push_back(Util::NetBytesToInt(*buf));
		(*buf) += 4;
	}
}

int PartitionMetadata::Size()
{
	int size = 0;
	size += 2 + 4 + 4;

	// array
	size += 4 + 4 * replicas_.size();

	// array
	size += 4 + 4 * isr_.size();

	return size;
}

void PartitionMetadata::Print()
{
	std::cout << "partition error code = " << partition_error_code_ << std::endl;
	std::cout << "partition id = " << partition_id_ << std::endl;
	std::cout << "leader = " << leader_ << std::endl;

	for (auto it = replicas_.begin(); it != replicas_.end(); ++it)
		std::cout << "replicas = " << *it << std::endl;

	for (auto it = isr_.begin(); it != isr_.end(); ++it)
		std::cout << "isr = " << *it << std::endl;
}

TopicMetadata::TopicMetadata(short error_code, const std::string &topic_name,
		const std::vector<PartitionMetadata> &partition_metadata)
{
	topic_error_code_ = error_code;
	topic_name_ = topic_name;
	partition_metadata_ = partition_metadata;
}

TopicMetadata::TopicMetadata(char **buf)
{
	// topic error code
	topic_error_code_ = Util::NetBytesToShort(*buf);
	(*buf) += 2;

	// topic name
	short topic_name_size = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	topic_name_ = std::string(*buf, topic_name_size);
	(*buf) += topic_name_size;

	// partition metadata array
	int partition_metadata_array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int j = 0; j < partition_metadata_array_size; j++)
	{
		PartitionMetadata partition_metadata(buf);
		partition_metadata_.push_back(partition_metadata);
	}
}

int TopicMetadata::Size()
{
	int size = 0;
	size += 2 + 2 + topic_name_.length();

	size += 4;
	for (auto it = partition_metadata_.begin(); it != partition_metadata_.end(); ++it)
		size += it->Size();

	return size;
}

void TopicMetadata::Print()
{
	std::cout << "topic error code = " << topic_error_code_ << std::endl;
	std::cout << "topic name = " << topic_name_ << std::endl;

	for (auto it = partition_metadata_.begin(); it != partition_metadata_.end(); ++it)
		it->Print();
}

MetadataResponse::MetadataResponse(int correlation_id, const std::vector<Broker> &brokers,
		const std::vector<TopicMetadata> &topic_metadata)
	: Response(ApiKey::MetadataRequest, correlation_id)
{
	brokers_ = brokers;
	topic_metadata_ = topic_metadata;
	total_size_ = Size();
}

MetadataResponse::MetadataResponse(char **buf)
	: Response(ApiKey::MetadataRequest, buf)
{
	// broker array
	int broker_array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int i = 0; i < broker_array_size; i++)
	{
		Broker broker(buf);
		brokers_.push_back(broker);
	}

	// topic metadata array
	int topic_metadata_array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int i = 0; i < topic_metadata_array_size; i++)
	{
		TopicMetadata topic_metadata(buf);
		topic_metadata_.push_back(topic_metadata);
	}

	if (total_size_ != Size())
	{
		throw "Size are not equal";
	}
}

int MetadataResponse::Size()
{
	int size = Response::Size();

	// array
	size += 4;
	for (auto it = brokers_.begin(); it != brokers_.end(); ++it)
		size += it->Size();

	// array
	size += 4;
	for (auto it = topic_metadata_.begin(); it != topic_metadata_.end(); ++it)
		size += it->Size();

	return size;
}

void MetadataResponse::Print()
{
	for (auto it = brokers_.begin(); it != brokers_.end(); ++it)
		it->Print();

	for (auto it = topic_metadata_.begin(); it != topic_metadata_.end(); ++it)
		it->Print();
}

