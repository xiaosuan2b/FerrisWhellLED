// #include <ArduinoSTL.h>

class HNO3Scheculer
{
    /** 非阻塞式调度
     * @author 刘向奥 <a3109809244@163.com>
     * 
     * 根据系统计时器，间隔指定时间调度线程
     */
private:
    void (*tar_func)(); // 需要调度的函数指针

    unsigned int duration;  // 调度间隔

    unsigned int sleep_duration; // sleep 时间

    unsigned long lastUpdate = 0; // 上次调度时间戳

public:
    HNO3Scheculer(void (*_tar_func)(), unsigned int _duration, 
                    unsigned int _sleep_duration = 0);

    void run(unsigned long currentMillis); // 放入 loop() 中，维持调度器运行

    void sleep_with_duration(unsigned int _sleep_duration); // sleep 线程

    void set_duration(unsigned int _duration); // 更改调度间隔


};


