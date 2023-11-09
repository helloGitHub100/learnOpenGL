// 对PortAudio和SoundTouch进行封装的测试程序，更方便用来播放音频文件
#include <iostream>
#include <portaudio.h>
#include <sndfile.h>
#include <mutex>
#include "AudioEngine.hpp"

int main()
{
    AudioEngine engine(RESOURCES_DIR"/audio/breakout.mp3"), engine1(RESOURCES_DIR"/audio/powerup.wav");
    PaError err = engine.start(true);
    if (err == paNoError)
    {
        // 等待音频播放完毕或等待按下Enter键退出
        // while (engine.isActive() && g_bKeyPressed == false)
        // {
        //     Pa_Sleep(100);
        // }
        Pa_Sleep(2000);
        engine1.start(false);
        Pa_Sleep(2000);
        engine1.start(false);
        std::cout << "[main] Playing audio. Press Enter to stop..." << std::endl;
        std::cin.get();
    }

    return 0;
}
