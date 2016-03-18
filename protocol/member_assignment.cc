#include <iostream>
#include <string.h>
#include <arpa/inet.h>

#include "member_assignment.h"
#include "util.h"

PartitionAssignment::PartitionAssignment(const std::string &topic, const std::vector<int> &partitions)
{
	topic_ = topic;
	partitions_ = partitions;
}

PartitionAssignment::PartitionAssignment(char **buf)
{
	// topic
	short topic_size = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	topic_ = std::string(*buf, topic_size);
	(*buf) += topic_size;

	// Partition array
	int partition_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int i = 0; i < partition_size; i++)
	{
		int partition = Util::NetBytesToInt(*buf);
		partitions_.push_back(partition);
		(*buf) += 4;
	}
}

int PartitionAssignment::CountSize()
{
	int size = 2 + topic_.length();

	// Partition array
	size += 4 + 4 * partitions_.size();
	return size;
}

void PartitionAssignment::PrintAll()
{
	std::cout << "topic = " << topic_ << std::endl;
	for (auto p_it = partitions_.begin(); p_it != partitions_.end(); ++p_it)
	{
		std::cout << "partition = " << *p_it << std::endl;
	}
}

void PartitionAssignment::Package(char **buf)
{
	// topic
	short topic_size = htons((short)topic_.length());
	memcpy(*buf, &topic_size, 2);
	(*buf) += 2;
	memcpy(*buf, topic_.c_str(), topic_.length());
	(*buf) += topic_.length();

	// Partition array
	int partition_size = htonl(partitions_.size());
	memcpy(*buf, &partition_size, 4);
	(*buf) += 4;
	for (auto p_it = partitions_.begin(); p_it != partitions_.end(); ++p_it)
	{
		int partition = htonl(*p_it);
		memcpy(*buf, &partition, 4);
		(*buf) += 4;
	}
}

MemberAssignment::MemberAssignment()
{

}

MemberAssignment::MemberAssignment(const std::string &topic, const std::vector<int> &partitions)
{
	version_ = 0;

	// only one topic
	for (int i = 0; i < 1; i++)
	{
		PartitionAssignment partition_assignment(topic, partitions);
		partition_assignment_.push_back(partition_assignment);
	}

	// user_data_ is empty 
}

MemberAssignment::MemberAssignment(char **buf)
{
	version_ = Util::NetBytesToShort(*buf);
	(*buf) += 2;

	// PartitionAssignment array
	int partition_assignment_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int i = 0; i < partition_assignment_size; i++)
	{
		PartitionAssignment partition_assignment(buf);
		partition_assignment_.push_back(partition_assignment);
	}

	// user data
	int user_data_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	user_data_ = std::string(*buf, user_data_size);
	(*buf) += user_data_size;
}

int MemberAssignment::CountSize()
{
	int size = 0;
	size += 2;

	// PartitionAssignment array
	size += 4;
	for (auto pa_it = partition_assignment_.begin(); pa_it != partition_assignment_.end(); ++pa_it)
	{
		size += pa_it->CountSize();
	}

	// UserData bytes
	size += 4 + user_data_.size();

	return size;
}

void MemberAssignment::PrintAll()
{
	std::cout << "version = " << version_ << std::endl;
	for (auto pa_it = partition_assignment_.begin(); pa_it != partition_assignment_.end(); ++pa_it)
	{
		pa_it->PrintAll();
	}
	std::cout << "user data = " << user_data_ << std::endl;
}

void MemberAssignment::Package(char **buf)
{
	// version
	short version = htons(version_);
	memcpy(*buf, &version, 2);
	(*buf) += 2;

	// PartitionAssignment array
	int partition_assignment_size = htonl(partition_assignment_.size());
	memcpy(*buf, &partition_assignment_size, 4);
	(*buf) += 4;
	for (auto pa_it = partition_assignment_.begin(); pa_it != partition_assignment_.end(); ++pa_it)
	{
		pa_it->Package(buf);
	}
	
	// user data
	int user_data_size = htonl(user_data_.size());
	memcpy(*buf, &user_data_size, 4);
	(*buf) += 4;
	memcpy(*buf, user_data_.data(), user_data_.size());
}