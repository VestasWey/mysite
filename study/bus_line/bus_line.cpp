// main.cpp 
//

#include <unistd.h>

//#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <list>
#include <map>
#include <memory>
#include <vector>
#include <iostream>
#include <random>
#include <stdio.h>

namespace
{
    int random(int max_num)
    {
        srand((int)time(0));
        return (rand() % max_num);
    }

    class HeartBeatDelegate
    {
    public:
        virtual void OnHeartBeatPerSecond(int total_secs) = 0;

    private:
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
    typedef std::list<Passenger> PassengerVct;
    std::map<int, std::pair<PassengerVct，PassengerVct>> g_pending_users; 

    class Driver
    {
    public:
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

        int up_drct_pause_time_in_sec = 3;
        int down_drct_pause_time_in_sec = 3;
    };

    const std::vector<BusStation> kStations
    {
        { 0, 5, 0 },
        { 0, 6, 4 },
        { 0, 7, 7 },
        { 0, 8, 5 },
        { 0, 4, 6 },

        { 0, 3, 3 },
        { 0, 6, 4 },
        { 0, 5, 5 },
        { 0, 6, 3 },
        { 0, 7, 7 },

        { 0, 4, 4 },
        { 0, 3, 5 },
        { 0, 6, 4 },
        { 0, 3, 5 },
        { 0, 0, 4 },
    };


    enum BusStatus
    {
        Depart,
        Running,
        Pause,
    };

    struct RunningStatus
    {
        RunningStatus(BusStatus status)
        {
            this->status = status;
            start_ts = std::chrono::steady_clock::now();
        }

        BusStatus status = BusStatus::Depart;
        std::chrono::steady_clock::time_point start_ts;
        std::chrono::steady_clock::time_point end_ts;

        PassengerVct surplus_users;

        int from = 0;
        int to = 0;
    };

    class BusManager;
    class Bus : public HeartBeatDelegate
    {
    public:
        Bus(const std::string& car_num, int start_ts)
            : car_num_(car_num)
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

    private:
        void AddStatus(RunningStatus& status, bool new_loop = false)
        {
            if (!status_.empty() && !new_loop)
            {
                RunningStatus& last_status = status_.back();
                last_status.end_ts = status.start_ts;
                status.from = last_status.to;
                status.to = (drct_ == Direction::Up) ? status.from + 1 : status.from - 1;
            }
            status_.push_back(status);
        }
        
        void OnHeartBeatPerSecond(int total_secs) override
        {
            // 检查车辆当前是否到站了，到站了可以上下车
            // 到站的判断是当前时间减去上个站的时间是否等于预设的该车在这两个站点间的运行时间
            std::chrono::seconds secs(total_secs - last_ts_);
            std::chrono::minutes dua = std::chrono::duration_cast<std::chrono::minutes>(
                secs);

            // 拿到上一个站点的信息，拿到时刻表上它到下个站的预设时间
            RunningStatus& last_status = status_.back();
            if (last_status.status == BusStatus::Depart)
            {
                // 初始发车，只能上客
                if (!g_pending_users[last_status.from].first.empty())
                {
                    last_status.surplus_users.push_back(g_pending_users[last_status.from].first.front());
                    g_pending_users[last_status.from].first.pop();
                }

                // 从当前的站点的等待用户中取用户上车，每人耗时一秒
                if (g_pending_users[last_status.from].first.empty())
                {
                    // 没人了，开车
                    last_status.status = BusStatus::Running;

                    last_ts_ = total_secs;
                }
            }
            else
            {
                int def_period = 0;
                if (drct_ == Direction::Up)
                {
                    def_period = kStations.at(last_status.from).up_drct_pause_time_in_sec;
                }
                else
                {
                    def_period = kStations.at(last_status.from).down_drct_pause_time_in_sec;
                }

                // 每次心跳都重新计算一下当前的路程估时，快慢误差在1分钟以内
                int tp = random(3);
                if (tp == 1)
                {
                    def_period -= 1;
                }
                else if (tp == 2)
                {
                    def_period += 1;
                }

                // 到点，到站，上下客，先下后上
                if (dua.count() >= def_period)
                {
                    last_status.status = BusStatus::Pause;

                    // 先把所有要在本站下车的人放下
                    auto iter = std::find_if(last_status.surplus_users.begin(),
                        last_status.surplus_users.end(), [&](const Passenger& user)->bool{
                            return (last_status.to == user.to);
                        });
                    if (iter != last_status.end())
                    {
                        last_status.surplus_users.erase(iter);
                    }
                    else
                    {
                        // 在当前到达站点等待上车的乘客开始上车，每人耗时一秒
                        PassengerVct* vct = nullptr;
                        if (drct_ == Direction::Up)
                        {
                            vct = &g_pending_users[last_status.to].first;
                        }
                        else
                        {
                            vct = &g_pending_users[last_status.to].second;
                        }
                        if (!vct->empty())
                        {
                            last_status.surplus_users.push_back(vct->front());
                            vct->pop();
                        }

                        if (vct->empty())
                        {
                            // 没人了，这个站是不是终点站，是的话调头
                            bool turn_around = false;
                            if (drct_ == Direction::Up)
                            {
                                turn_around = (last_status.to == kStations.size() - 1);
                            }
                            else
                            {
                                turn_around = (last_status.to == 0);
                            }

                            // 没人了，开车
                            if (!turn_around)
                            {
                                RunningStatus status(BusStatus::Running);
                                AddStatus(status);
                            }
                            else
                            {
                                SetDirection((drct_ == Direction::Up) ? Direction::Down : Direction::Up);

                                RunningStatus status(BusStatus::Depart);
                                status.from = (drct_ == Direction::Up) ? 0 : kStations.size() - 1;
                                status.to = (drct_ == Direction::Up) ? status.from + 1 : status.from - 1;
                                
                                AddStatus(status, true);
                            }

                            last_ts_ = total_secs;
                        }
                    }
                }
            }
        }

    private:
        friend class BusManager;
        std::string car_num_;

        int last_ts_ = 0;
        Direction drct_ = Direction::Up;

        Driver driver_;
        std::list<RunningStatus> status_;
    };

    class BusManager : public HeartBeatDelegate
    {
    public:
        BusManager()
        {
        }

        void PrintBusStatus(const std::string& car_num)
        {
            // TODO
        }

    private:
        void OnHeartBeatPerSecond(int total_secs) override
        {
            // 每15分钟各方向各自发一辆车
            std::chrono::seconds secs(total_secs - last_ts_);
            std::chrono::minutes dua = std::chrono::duration_cast<std::chrono::minutes>(
                secs);
            static int bus_count = 5;
            if (bus_count > 0 && (dua.count() == 15 || dua.count() == 0))
            {
                last_ts_ = total_secs;

                AddBus(std::to_string(5 - bus_count) + "_up", 0, total_secs);
                AddBus(std::to_string(5 - bus_count) + "_down", kStations.size() - 1, total_secs);

                --bus_count;
            }
        }

        void AddBus(const std::string& car_num, int from_idx, int total_secs)
        {
            std::unique_ptr<Bus> bus = std::make_unique<Bus>(car_num, total_secs);

            RunningStatus status(BusStatus::Depart);
            status.from = from_idx;
            status.to = (from_idx == 0) ? from_idx + 1 : from_idx - 1;
            bus->SetDirection((from_idx == 0) ? Direction::Up : Direction::Down);
            bus->AddStatus(status);

            // 新发车，让其开始进行上客
            bus->OnHeartBeatPerSecond(total_secs);

            buses_[car_num] = std::move(bus);
        }

    private:
        int last_ts_ = 0;
        std::map<std::string, std::unique_ptr<Bus>> buses_;
    };

    class Tester : public HeartBeatDelegate
    {
    public:
        Tester()
        {
        }
        ~Tester() = default;

    private:
        void OnHeartBeatPerSecond(int total_secs) override
        {
            // 每5分钟产生10个乘客，方向随机，目标站点随机
            std::chrono::seconds secs(total_secs - last_ts_);
            std::chrono::minutes dua = std::chrono::duration_cast<std::chrono::minutes>(
                secs);
            if (dua.count() == 5)
            {
                last_ts_ = total_secs;

                static int user_id = 0;
                for (size_t i = 0; i < 10; i++)
                {
                    int from_idx = random((int)kStations.size());
                    int to_idx = random((int)kStations.size());
                    while (from_idx != to_idx)
                    {
                        to_idx = random((int)kStations.size());
                    }
                    Passenger user;
                    user.id = ++user_id;
                    user.from = from_idx;
                    user.to = to_idx;
                    user.drct = (from_idx < to_idx) ? Direction::Up : Direction::Down;

                    if (user.drct == Direction::Up)
                    {
                        g_pending_users[from_idx].first.push_back(std::move(user));
                    }
                    else
                    {
                        g_pending_users[from_idx].second.push_back(std::move(user));
                    }
                }
            }
        }
    
    private:
        int last_ts_ = 0;
    };
}


int main(int argc, char* argv[])
{
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
        for (auto& delegate : g_observers)
        {
            delegate->OnHeartBeatPerSecond(++ts);
        }
    }
    
	return 0;
}

