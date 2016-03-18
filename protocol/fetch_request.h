#ifndef _FETCH_REQUEST_H_
#define _FETCH_REQUEST_H_

#include <string>

#include "request.h"

class FetchRequest: public Request {
public:
	FetchRequest(const std::string &topic_name, int partition, long fetch_offset, int correlation_id = ApiKey::FetchType);
	virtual ~FetchRequest() {}

	virtual int CountSize();
	virtual void PrintAll();
	virtual void Package(char **buf);

	int         replica_id_;
	int         max_wait_time_;
	int         min_bytes_;
	std::string topic_name_;
	int         partition_;
	long        fetch_offset_;
	int         max_bytes_;
};

#endif