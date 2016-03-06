#ifndef _MESSAGE_SETS_H_
#define _MESSAGE_SETS_H_

#include <stdint.h>
#include <string>
#include <vector>

class Message {
public:
	Message();
	Message(char **buf);

	int CountSize();
	void PrintAll();

	int crc_;
	int8_t magic_byte_;
	int8_t attributes_;
	std::string key_;	// bytes
	std::string value_;	// bytes
};

class OffsetAndMessage {
public:
	OffsetAndMessage();
	OffsetAndMessage(char **buf);

	int CountSize();
	void PrintAll();

	long offset_;
	int message_size_;
	Message message_;
};

class MessageSet {
public:
	MessageSet();
	MessageSet(char **buf);

	int CountSize();
	void PrintAll();

	// array
	std::vector<OffsetAndMessage> offset_message_;
};

#endif