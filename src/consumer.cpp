#include "consumer.h"
#include "exceptions.h"
#include "configuration.h"
#include "topic_partition_list.h"

using std::vector;
using std::string;

namespace cppkafka {

Consumer::Consumer(const Configuration& config) {
    char error_buffer[512];
    rd_kafka_t* ptr = rd_kafka_new(RD_KAFKA_CONSUMER, config.get_handle(),
                                   error_buffer, sizeof(error_buffer));
    if (!ptr) {
        throw Exception("Failed to create consumer handle: " + string(error_buffer));
    }
    set_handle(ptr);
}

void Consumer::subscribe(const vector<string>& topics) {
    TopicPartitionList list(topics.begin(), topics.end());
    rd_kafka_resp_err_t error = rd_kafka_subscribe(get_handle(), list.get_handle());
    check_error(error);
}

void Consumer::unsubscribe() {
    rd_kafka_resp_err_t error = rd_kafka_unsubscribe(get_handle());
    check_error(error);
}

void Consumer::assign(const TopicPartitionList& topic_partitions) {
    // If the list is empty, then we need to use a null pointer
    auto handle = topic_partitions.empty() ? nullptr : topic_partitions.get_handle();
    rd_kafka_resp_err_t error = rd_kafka_assign(get_handle(), handle);
    check_error(error);
}

void Consumer::check_error(rd_kafka_resp_err_t error) {
    if (error != RD_KAFKA_RESP_ERR_NO_ERROR) {
        throw HandleException(error);
    }
}

} // cppkafka