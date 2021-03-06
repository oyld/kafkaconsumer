#include <arpa/inet.h>
#include "sync_group_request.h"
#include "easylogging++.h"

GroupAssignment::GroupAssignment(const std::string &topic, const std::string &member_id,
		const std::vector<int> &partitions)
	: member_assignment_(topic, partitions)
{
	member_id_ = member_id;
}

int GroupAssignment::CountSize()
{
	int size = 0;
	size += 2 + member_id_.length() +
			4 + member_assignment_.CountSize();

	return size;
}

void GroupAssignment::PrintAll()
{
	LOG(DEBUG) << "member id = " << member_id_;
	member_assignment_.PrintAll();
}

void GroupAssignment::Package(char **buf)
{
	// member id
	short member_id_size = htons((short)member_id_.length());
	memcpy(*buf, &member_id_size, 2);
	(*buf) += 2;
	memcpy(*buf, member_id_.c_str(), member_id_.length());
	(*buf) += member_id_.length();

	// MemberAssignment bytes
	int member_assignment_size = htonl(member_assignment_.CountSize());
	memcpy(*buf, &member_assignment_size, 4);
	(*buf) += 4;
	member_assignment_.Package(buf);
}

//----------------------------------------
SyncGroupRequest::SyncGroupRequest(const std::string &topic, const std::string &group_id,
		int generation_id, const std::string &member_id,
		const std::unordered_map<std::string, std::vector<int>> &member_partition_map, int correlation_id)
	: Request(ApiKey::SyncGroupType, correlation_id)
{
	group_id_ = group_id;
	generation_id_ = generation_id;
	member_id_ = member_id;

	for (auto mp_it = member_partition_map.begin(); mp_it != member_partition_map.end(); ++mp_it)
	{
		GroupAssignment group_assignment(topic, mp_it->first, mp_it->second);
		group_assignment_.push_back(group_assignment);
	}

	total_size_ = CountSize();
}

SyncGroupRequest::SyncGroupRequest(const std::string &topic, const std::string &group_id,
		int generation_id, const std::string &member_id, int correlation_id)
	: Request(ApiKey::SyncGroupType, correlation_id)
{
	group_id_ = group_id;
	generation_id_ = generation_id;
	member_id_ = member_id;
	total_size_ = CountSize();
}
int SyncGroupRequest::CountSize()
{
	int size = Request::CountSize();
	size += 2 + group_id_.length() +	// group id
			4 +		// generation id
			2 + member_id_.length(); 	// member id

	// GroupAssignment array
	size += 4;
	for (auto ga_it = group_assignment_.begin(); ga_it != group_assignment_.end(); ++ga_it)
	{
		size += ga_it->CountSize();
	}

	return size;
}

void SyncGroupRequest::PrintAll()
{
	LOG(DEBUG) << "-----SyncGroupRequest-----";
	Request::PrintAll();
	LOG(DEBUG) << "group id = " << group_id_;
	LOG(DEBUG) << "generation id = " << generation_id_;
	LOG(DEBUG) << "member id = " << member_id_;
	for (auto ga_it = group_assignment_.begin(); ga_it != group_assignment_.end(); ++ga_it)
	{
		ga_it->PrintAll();
	}
	LOG(DEBUG) << "-------------------------";
}

void SyncGroupRequest::Package(char **buf)
{
	Request::Package(buf);

	// group id
	short group_id_len = htons((short)group_id_.length());
	memcpy(*buf, &group_id_len, 2);
	(*buf) += 2;
	memcpy(*buf, group_id_.c_str(), group_id_.length());
	(*buf) += group_id_.length();

	// generation id
	int generation_id = htonl(generation_id_);
	memcpy(*buf, &generation_id, 4);
	(*buf) += 4;

	// member id
	short member_id_size = htons((short)member_id_.length());
	memcpy(*buf, &member_id_size, 2);
	(*buf) += 2;
	memcpy(*buf, member_id_.c_str(), member_id_.length());
	(*buf) += member_id_.length();

	// group assignment array
	int group_assignment_size = htonl(group_assignment_.size());
	memcpy(*buf, &group_assignment_size, 4);
	(*buf) += 4;

	for (auto ga_it = group_assignment_.begin(); ga_it != group_assignment_.end(); ++ga_it)
	{
		ga_it->Package(buf);
	}
}
