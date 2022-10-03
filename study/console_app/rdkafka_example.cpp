#include "stdafx.h"

#include "librdkafka/src-cpp/rdkafkacpp_int.h"
#include "observer_list.h"

struct RdKafkaTypeDeleter
{
    void operator()(void* ptr) const
    {
        if (ptr)
        {
            RdKafka::mem_free(ptr);
        }
    }
};

template<class KafkaType>
using RdKafkaPtr = std::unique_ptr<KafkaType, RdKafkaTypeDeleter>;

template<class KafkaType>
RdKafkaPtr<KafkaType> MakeRdKafkaPtr(KafkaType* raw_ptr)
{
    RdKafkaPtr<KafkaType> ptr(raw_ptr);
    return ptr;
}


#define CHECK_RDKAFKA_RESULT(func, errlog, result) \
    if (0 != func)  \
    {   \
        printf(errlog);  \
        return result;  \
    }


class RdKafkaProxyObserver
{
public:
    virtual void OnRecv(const std::string& topic, const std::string& msg) {}
    virtual void OnSend(const std::string& topic, const std::string& msg) {}

};

class RdKafkaProxy
{
public:
    void AddObserver(RdKafkaProxyObserver* obs)
    {
        observer_list_.AddObserver(obs);
    }
    void RemoveObserver(RdKafkaProxyObserver* obs)
    {
        observer_list_.RemoveObserver(obs);
    }
    void Notify()
    {
        FOR_EACH_OBSERVER(RdKafkaProxyObserver, observer_list_, OnRecv("asd", "123"));
    }

    int DoSth(int i)
    {
        return i;
    }

private:
    ObserverList<RdKafkaProxyObserver> observer_list_;
};

class ObserverImpl : public RdKafkaProxyObserver
{
public:
    ObserverImpl(int i) : i_(i) {}

    void OnRecv(const std::string& topic, const std::string& msg) override {
        printf("OnRecv i=%d, topic=%s, msg=%s", i_, topic.c_str(), msg.c_str());
    }
    void OnSend(const std::string& topic, const std::string& msg) override {
        printf("OnSend i=%d, topic=%s, msg=%s", i_, topic.c_str(), msg.c_str());
    }

private:
    int i_;
};


void rdkafka_example()
{
    //RdKafkaPtr<RdKafka::Conf> conf = MakeRdKafkaPtr<RdKafka::Conf>(RdKafka::ConfImpl::create(RdKafka::Conf::CONF_TOPIC));
//     RdKafkaPtr<RdKafka::Conf> conf(RdKafka::ConfImpl::create(RdKafka::Conf::CONF_TOPIC));
//     std::string errstr;
//     RdKafka::Conf::ConfResult ret;
//     ret = conf->set("metadata.broker.list", "192.168.1.111:9802", errstr);
//     ret = conf->set("group.id", "AdmsXXX", errstr);
//     ret = conf->set("debug", "broker,topic,msg,consumer,cgrp,topic,fetch", errstr);
//     ret = conf->set("statistics.interval.ms", "10", errstr);    // Í³¼Æ¼ä¸ô

    //RdKafkaPtr<RdKafka::KafkaConsumer> consumer = MakeRdKafkaPtr(RdKafka::KafkaConsumerImpl::create(conf.get(), errstr));
    //RdKafka::ErrorCode err = consumer->subscribe(topics);

    RdKafkaProxy proxy;
    ObserverImpl obs(22);
    proxy.AddObserver(&obs);
    ObserverImpl obs1(33);
    proxy.AddObserver(&obs1);
    proxy.Notify();
    proxy.RemoveObserver(&obs);
    proxy.RemoveObserver(&obs1);

    CHECK_RDKAFKA_RESULT(proxy.DoSth(0), "proxy.DoSth(0)", void(0));
    CHECK_RDKAFKA_RESULT(proxy.DoSth(1), "proxy.DoSth(1)", void(0));
    CHECK_RDKAFKA_RESULT(proxy.DoSth(0), "proxy.DoSth(000)", void(0));

}