#ifndef PADT_BREP_UTILITY_H
#define PADT_BREP_UTILITY_H

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/message_lite.h>
namespace padt {
namespace pbio {
bool readDelimitedFrom(
	google::protobuf::io::ZeroCopyInputStream* rawInput,
	google::protobuf::MessageLite* message) {
	// We create a new coded stream for each message.  Don't worry, this is fast,
	// and it makes sure the 64MB total size limit is imposed per-message rather
	// than on the whole stream.  (See the CodedInputStream interface for more
	// info on this limit.)
	google::protobuf::io::CodedInputStream input(rawInput);

	// Read the size.
	uint32_t size;
	if (!input.ReadVarint32(&size)) return false;

	// Tell the stream not to read beyond that size.
	google::protobuf::io::CodedInputStream::Limit limit =
		input.PushLimit(size);

	// Parse the message.
	if (!message->MergeFromCodedStream(&input)) return false;
	if (!input.ConsumedEntireMessage()) return false;

	// Release the limit.
	input.PopLimit(limit);

	return true;
}
}
}
#endif
