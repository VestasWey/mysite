#include "stdafx.h"

#ifdef _WIN32
#include <conio.h>
#endif

//#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <list>
#include <map>
#include <memory>
#include <vector>
#include <iosfwd>
#include <random>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "def.h"

const int c_int = 1;
static int s_int = 1;
int g_int = 1;

namespace
{
    int random(int max_num)
    {
        static bool init = true;
        if (init)
        {
            init = false;
            srand((int)time(0));
        }
        return (rand() % max_num);
    }

    class HeartBeatDelegate
    {
    public:
        virtual void OnHeartBeatPerSecond(int total_secs) = 0;

    protected:
        virtual ~HeartBeatDelegate() = default;
    };
    std::list<HeartBeatDelegate*> g_observers;

    enum Direction
    {
        Up,
        Down,
    };

    struct Passenger
    {
        int id = 0;
        Direction drct = Direction::Up;
        int from = 0;
        int to = 0;
    };
    typedef std::list<Passenger> PassengerList;
    std::map<int, PassengerList> g_up_drct_users;     // 上行方向各站点后车乘客
    std::map<int, PassengerList> g_down_drct_users;   // 下行方向各站点后车乘客

    class Driver
    {
    public:
        Driver() = default;
        Driver(const std::string& name)
            : name_(name)
        {
        }

    private:

        std::string name_;
    };

    struct BusStation
    {
        int num = 0;

        int up_drct_to_next_station_time_in_minutes = 3;    // 上行时，该站点开到下一站点的预估耗时
        int down_drct_to_next_station_time_in_minutes = 3;  // 下行时，该站点开到下一站点的预估耗时
    };

    // 所有站点信息
    const std::vector<BusStation> kStations
    {
        { 0, 5, 0 },
        { 1, 6, 4 },
        { 2, 7, 7 },
        { 3, 8, 5 },
        { 4, 4, 6 },

        { 5, 3, 3 },
        { 6, 6, 4 },
        { 7, 5, 5 },
        { 8, 6, 3 },
        { 9, 7, 7 },

        { 10, 4, 4 },
        { 11, 3, 5 },
        { 12, 6, 4 },
        { 13, 3, 5 },
        { 14, 0, 4 },
    };

    enum BusStatus
    {
        Depart, // 发车，待上客状态
        Running,// 行驶状态
        Pause,  // 中途停车上下客状态
        ToEnd,  // 抵达终点
    };

    struct RunningStatus
    {
        RunningStatus() = default;
        RunningStatus(BusStatus status, int start_ts, Direction drct)
        {
            this->status = status;
            this->start_ts = start_ts;
            this->drct = drct;
            //start_ts = std::chrono::steady_clock::now();
        }

        std::string ToString()
        {
            std::ostringstream ss;
            ss << "start_ts:" << start_ts 
                << ", end_ts:" << end_ts 
                << ", status:" << status
                << ", user_count:" << surplus_users.size()
                << ", drct:" << drct
                << ", from:" << from
                << ", to:" << to
                << std::endl;
            return ss.str();
        }

        BusStatus status = BusStatus::Depart;
        //std::chrono::steady_clock::time_point start_ts;
        //std::chrono::steady_clock::time_point end_ts;
        int start_ts = 0;
        int end_ts = 0;

        PassengerList surplus_users; // 该状态下，车上乘客列表

        Direction drct = Direction::Up;
        int from = 0;
        int to = 0;
    };

    class BusManager;
    class Bus : public HeartBeatDelegate
    {
    public:
        Bus(const std::string& car_num, int num, int start_ts)
            : car_num_(car_num),
            num_(num)
        {
            g_observers.push_back(this);
            last_ts_ = start_ts;
        }

        ~Bus()
        {
            g_observers.remove(this);
        }

        void SetDirection(Direction drct)
        {
            drct_ = drct;
        }

        void SetDriver(Driver driver)
        {
            driver_ = driver;
        }

        void PrintBusStatus()
        {
            std::cout << "[bus " << num_ << "] car_num:" << car_num_ << ", until now status infos:" << std::endl;
            for (auto& iter : status_)
            {
                std::cout << "\t" << iter.ToString();
            }
            std::cout << std::endl;
        }

    private:
        void AddStatus(RunningStatus& status)
        {
            switch (status.status)
            {
            case BusStatus::Running:
                if (current_status_.status == BusStatus::Depart)
                {
                    status.from = current_status_.from;
                    status.to = current_status_.to;
                }
                else
                {
                    // 常规状态变为行驶状态就是中途靠站上下客之后的再次开车
                    status.from = current_status_.to;
                    status.to = (drct_ == Direction::Up) ? status.from + 1 : status.from - 1;
                }
                status.surplus_users = current_status_.surplus_users;
                break;
            case BusStatus::Pause:
                status.from = current_status_.from;
                status.to = current_status_.to;
                status.surplus_users = current_status_.surplus_users;
                break;
            case BusStatus::ToEnd:
                status.from = current_status_.from;
                status.to = current_status_.to;
                break;
            default:
                break;
            }

            if (!status_.empty())
            {
                auto& value = status_.back();
                value.end_ts = status.start_ts;
            }

            status_.push_back(status);
            current_status_ = status;
        }
        
        void OnHeartBeatPerSecond(int total_secs) override
        {
            // 拿到上一个站点的信息，拿到时刻表上它到下个站的预设时间
            if (current_status_.status == BusStatus::Depart)
            {
                // 初始发车，只能上客
                PassengerList* vct = nullptr;
                if (drct_ == Direction::Up)
                {
                    vct = &g_up_drct_users[current_status_.from];
                }
                else
                {
                    vct = &g_down_drct_users[current_status_.from];
                }

                // 从当前的站点的等待用户中取用户上车，每人耗时一秒
                if (!vct->empty())
                {
                    std::cout << "[bus] num:" << num_
                        << ", car_num:" << car_num_
                        << ", drct_:" << drct_
                        << ", status:" << current_status_.status
                        << ", pick passenger:" << vct->front().id
                        << std::endl;

                    current_status_.surplus_users.push_back(vct->front());
                    vct->pop_front();
                }

                if (vct->empty())
                {
                    std::cout << "[bus] num:" << num_
                        << ", car_num:" << car_num_
                        << ", drct_:" << drct_
                        << ", users:" << current_status_.surplus_users.size()
                        << ", running..."
                        << std::endl;

                    // 没人了，开车
                    RunningStatus status(BusStatus::Running, total_secs, drct_);
                    AddStatus(status);

                    last_ts_ = total_secs;
                }
            }
            else
            {
                // 每次心跳都重新计算一下当前的路程估时，快慢误差在1分钟以内
                if (current_status_.status != BusStatus::Pause)
                {
                    int def_period = 0;
                    if (drct_ == Direction::Up)
                    {
                        def_period = kStations.at(current_status_.from).up_drct_to_next_station_time_in_minutes;
                    }
                    else
                    {
                        def_period = kStations.at(current_status_.from).down_drct_to_next_station_time_in_minutes;
                    }

                    int tp = random(3);
                    if (tp == 1)
                    {
                        def_period -= 1;
                    }
                    else if (tp == 2)
                    {
                        def_period += 1;
                    }

                    // 检查车辆当前是否到站了，到站了可以上下车
                    // 到站的判断是当前时间减去上个站的时间是否等于预设的该车在这两个站点间的运行时间
                    std::chrono::seconds secs(total_secs - last_ts_);
                    std::chrono::minutes dua = std::chrono::duration_cast<std::chrono::minutes>(
                        secs);

                    // 到站停车，准备上下客
                    if (dua.count() >= def_period)
                    {
                        std::cout << "[bus] num:" << num_
                            << ", car_num:" << car_num_
                            << ", drct_:" << drct_
                            << ", users:" << current_status_.surplus_users.size()
                            << ", arrived at station:" << current_status_.to
                            << ", with error time type:" << tp
                            << std::endl;

                        RunningStatus status(BusStatus::Pause, total_secs, drct_);
                        AddStatus(status);
                    }
                }

                // 到点，到站，上下客，先下后上
                if (current_status_.status == BusStatus::Pause)
                {
                    // 先把所有要在本站下车的人放下，每人耗时一秒
                    auto iter = std::find_if(current_status_.surplus_users.begin(),
                        current_status_.surplus_users.end(), [&](const Passenger& user)->bool{
                            return (current_status_.to == user.to);
                        });
                    if (iter != current_status_.surplus_users.end())
                    {
                        std::cout << "[bus] num:" << num_
                            << ", car_num:" << car_num_
                            << ", drct_:" << drct_
                            << ", status:" << current_status_.status
                            << ", drop off passenger:" << iter->id
                            << std::endl;

                        current_status_.surplus_users.erase(iter);
                        // TODO：记录下都哪些人下车了
                        //////////////////////////////////////////////////////////////////////////
                    }
                    else
                    {
                        // 在当前到达站点等待上车的乘客开始上车，每人耗时一秒
                        PassengerList* vct = nullptr;
                        if (drct_ == Direction::Up)
                        {
                            vct = &g_up_drct_users[current_status_.to];
                        }
                        else
                        {
                            vct = &g_down_drct_users[current_status_.to];
                        }
                        if (!vct->empty())
                        {
                            std::cout << "[bus] num:" << num_
                                << ", car_num:" << car_num_
                                << ", drct_:" << drct_
                                << ", status:" << current_status_.status
                                << ", pick passenger:" << vct->front().id
                                << std::endl;

                            current_status_.surplus_users.push_back(vct->front());
                            vct->pop_front();
                            // TODO：记录下都哪些人上车了
                            //////////////////////////////////////////////////////////////////////////
                        }

                        if (vct->empty())
                        {
                            // 没人了，这个站是不是终点站，是的话调头
                            bool turn_around = false;
                            if (drct_ == Direction::Up)
                            {
                                turn_around = (current_status_.to == kStations.size() - 1);
                            }
                            else
                            {
                                turn_around = (current_status_.to == 0);
                            }

                            // 没人了，开车
                            if (!turn_around)
                            {
                                RunningStatus status(BusStatus::Running, total_secs, drct_);
                                AddStatus(status);

                                last_ts_ = total_secs;
                            }
                            else
                            {
                                std::cout << "[bus] num:" << num_
                                    << ", car_num:" << car_num_
                                    << ", drct_:" << drct_
                                    << ", status:" << current_status_.status
                                    << ", arrived at the end, then turn arround."
                                    << std::endl;

                                {
                                    RunningStatus status(BusStatus::ToEnd, total_secs, drct_);
                                    AddStatus(status);
                                }

                                SetDirection((drct_ == Direction::Up) ? Direction::Down : Direction::Up);

                                RunningStatus status(BusStatus::Depart, total_secs, drct_);
                                status.from = (drct_ == Direction::Up) ? 0 : kStations.size() - 1;
                                status.to = (drct_ == Direction::Up) ? status.from + 1 : status.from - 1;
                                AddStatus(status);
                            }
                        }
                    }
                }
            }
        }

    private:
        friend class BusManager;
        int num_ = 0;
        std::string car_num_;

        int last_ts_ = 0;
        Direction drct_ = Direction::Up;
        RunningStatus current_status_;

        Driver driver_;
        std::list<RunningStatus> status_;
    };

    class BusManager : public HeartBeatDelegate
    {
    public:
        BusManager()
        {
        }

        void PrintBusStatus(int idx)
        {
            if (buses_.find(idx) != buses_.end())
            {
                buses_.at(idx)->PrintBusStatus();
            }
        }

        void PrintAllBusStatus()
        {
            for (auto& iter : buses_)
            {
                iter.second->PrintBusStatus();
            }
        }

    private:
        void OnHeartBeatPerSecond(int total_secs) override
        {
            // 每15分钟各方向各自发一辆车
            std::chrono::seconds secs(total_secs - last_ts_);
            std::chrono::minutes dua = std::chrono::duration_cast<std::chrono::minutes>(
                secs);
            static int bus_count = 5;
            static int bus_num = 0;
            if (bus_count > 0 && (dua.count() == 15 || (dua.count() == 0 && init_once_)))
            {
                init_once_ = false;
                last_ts_ = total_secs;

                std::cout << "generate 2 bus:" << std::endl;
                std::cout << "\tnum " << bus_num << "and " << bus_num + 1
                    << std::endl;

                AddBus(std::to_string(5 - bus_count) + "_up", 0, bus_num++, total_secs);
                AddBus(std::to_string(5 - bus_count) + "_down", kStations.size() - 1, bus_num++, total_secs);

                --bus_count;
            }
        }

        void AddBus(const std::string& car_num, int from_idx, int num, int total_secs)
        {
            std::unique_ptr<Bus> bus = std::make_unique<Bus>(car_num, num, total_secs);

            Direction drct = (from_idx == 0) ? Direction::Up : Direction::Down;
            RunningStatus status(BusStatus::Depart, total_secs, drct);
            status.from = from_idx;
            status.to = (from_idx == 0) ? from_idx + 1 : from_idx - 1;
            bus->SetDirection(drct);
            bus->AddStatus(status);

            // 新发车，让其开始进行上客
            bus->OnHeartBeatPerSecond(total_secs);

            buses_[num] = std::move(bus);
        }

    private:
        int last_ts_ = 0;
        std::map<int, std::unique_ptr<Bus>> buses_;
        bool init_once_ = true;
    };

    class Tester : public HeartBeatDelegate
    {
    public:
        Tester()
        {
        }
        ~Tester() = default;

        void PrintPassengerDistribution()
        {
            std::cout << "passenger_count" << passenger_count_ << std::endl;
            std::cout << "up_drct_users distribution: " << std::endl;
            for (auto& iter : up_drct_users_)
            {
                std::cout << "\tstation[" << iter.first << "]=" << iter.second << std::endl;
            }
            std::cout << "down_drct_users distribution: " << std::endl;
            for (auto& iter : down_drct_users_)
            {
                std::cout << "\tstation[" << iter.first << "]=" << iter.second << std::endl;
            }
            std::cout << "still have some up_drct_users waiting: " << std::endl;
            for (auto& iter : g_up_drct_users)
            {
                if (!iter.second.empty())
                {
                    std::cout << "\tstation[" << iter.first << "]=" << iter.second.size() << std::endl;
                }
            }
            std::cout << "still have some down_drct_users waiting: " << std::endl;
            for (auto& iter : g_down_drct_users)
            {
                if (!iter.second.empty())
                {
                    std::cout << "\tstation[" << iter.first << "]=" << iter.second.size() << std::endl;
                }
            }
        }

    private:
        void OnHeartBeatPerSecond(int total_secs) override
        {
            // 每5分钟产生10个乘客，方向随机，目标站点随机
            std::chrono::seconds secs(total_secs - last_ts_);
            std::chrono::minutes dua = std::chrono::duration_cast<std::chrono::minutes>(
                secs);
            if (dua.count() == 5 || init_once_)
            {
                init_once_ = false;
                last_ts_ = total_secs;

                std::cout << "generate 10 passengers:" << std::endl;
                static int user_id = 0;
                for (size_t i = 0; i < 10; i++)
                {
                    int from_idx = random((int)kStations.size());
                    int to_idx = random((int)kStations.size());
                    while (from_idx == to_idx)
                    {
                        to_idx = random((int)kStations.size());
                    }
                    Passenger user;
                    user.id = ++user_id;
                    user.from = from_idx;
                    user.to = to_idx;
                    user.drct = (from_idx < to_idx) ? Direction::Up : Direction::Down;

                    std::cout << "\tid:" << user.id 
                        << ", from:" << user.from
                        << ", to:" << user.to
                        << ", drct:" << user.drct
                        << std::endl;

                    passenger_count_++;

                    if (user.drct == Direction::Up)
                    {
                        g_up_drct_users[from_idx].push_back(std::move(user));
                        up_drct_users_[from_idx]++;
                    }
                    else
                    {
                        g_down_drct_users[from_idx].push_back(std::move(user));
                        down_drct_users_[from_idx]++;
                    }
                }
            }
        }
    
    private:
        int last_ts_ = 0;
        bool init_once_ = true;

        int passenger_count_ = 0;
        std::map<int, int> up_drct_users_;
        std::map<int, int> down_drct_users_;
    };
}

struct RValue {
    RValue() :sources("hello!!!") {}
    RValue(RValue&& a) {
        sources = std::move(a.sources);
        std::cout << "&& RValue" << std::endl;
    }

    RValue(const RValue& a) {
        sources = a.sources;
        std::cout << "& RValue" << std::endl;
    }

    void operator=(const RValue&& a) {
        sources = std::move(a.sources);
        std::cout << "&& ==" << std::endl;
    }

    void operator=(const RValue& a) {
        sources = a.sources;
        std::cout << "& ==" << std::endl;
    }

    std::string sources;;
};

RValue get() {
    RValue a;
    return a;
}

void put(RValue) {}

int rht() {
    RValue a = get();
    std::cout << "---------------" << std::endl;
    put(RValue());
    return 0;
}

void bus_line_example()
{
    p_init_val.s_counter_++;
    printf("bus_line_example print p_init_val\n");
    p_init_val.print();

    s_init_val.s_counter_--;
    printf("bus_line_example print s_init_val\n");
    s_init_val.print();

    printf("bus_line_example print c_init_val address %p \n", &c_init_val);

    printf("bus_line_example print e_init_val address %p \n", &e_init_val);

    //rht();
    return;
    std::cout << "Go Go Go!!!" << std::endl;

    Tester tester;
    BusManager bus_manager;

    int max_test_time = 300 * 60;
    g_observers.push_back(&tester);
    g_observers.push_back(&bus_manager);
    //std::chrono::steady_clock::time_point last_ts = std::chrono::steady_clock::now();
    int ts = 0;
    while (ts < max_test_time)
    {
        int key = -1;
#ifdef _WIN32
        if (_kbhit() != 0)
        {
            key = _getch();
        }
#endif // _WIN32

#ifdef __APPLE__
        system("stty -icanon");
        key = getchar();
#endif
        if (key == 0x1B)    // esc
        {
            std::cout << "exit bus_line_example." << std::endl;
            break;
        }
        else if (key >= 0x30 && key <= 0x39)
        {
            int idx = key - 0x30;
            bus_manager.PrintBusStatus(idx);
        }

        for (auto& delegate : g_observers)
        {
            delegate->OnHeartBeatPerSecond(++ts);
        }
    }

    bus_manager.PrintAllBusStatus();
    tester.PrintPassengerDistribution();

    std::cout << "bus_line_example exit." << std::endl;
}


#ifdef __APPLE__
int main(int argc, char* argv[]) {
    bus_line_example();
    return 0;
}
#endif
