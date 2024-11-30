#include "HNO3Scheculer.h"


HNO3Scheculer::HNO3Scheculer
    // 构造器
    (
    void (*_tar_func)(),
    unsigned int _duration, 
    unsigned int _sleep_duration
    )

    : tar_func(_tar_func),
        duration(_duration),
        sleep_duration(_sleep_duration)
    {}

void HNO3Scheculer::run(unsigned long currentMillis)
{
    /** 调度器主运行函数
     * 放入 loop() 中，维持调度器运行
     * @author 刘向奥 <a3109809244@163.com>
     */
    
    // 判断是否处于 sleep 状态
    if (currentMillis - lastUpdate < sleep_duration)
    {
        return;
    }else
    {
        sleep_duration = 0;
    }
    
    // 达到间隔时间，运行目标函数
    if (currentMillis - lastUpdate >= duration)
	{
		lastUpdate = currentMillis;
        tar_func();
	}
}


void HNO3Scheculer::set_duration(unsigned int _duration)
{
    duration = _duration;
}


void HNO3Scheculer::sleep_with_duration(unsigned int _sleep_duration)
{
    sleep_duration = _sleep_duration;
}