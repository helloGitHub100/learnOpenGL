// 对PortAudio和SoundTouch进行封装的测试程序，更方便用来播放音频文件
#include <iostream>
#include <portaudio.h>
#include <sndfile.h>
#include <mutex>

// 音频引擎类，对PortAudio和SoundTouch进行封装
class AudioEngine
{
public:
    AudioEngine(const char* filePath)
    {
        m_filePath = filePath;
        constructFunc();
    }
    // 拷贝构造函数
    AudioEngine(const AudioEngine& other)
    {
        m_filePath = other.m_filePath;
        m_bLoop = other.m_bLoop;
        constructFunc();
    }
    // =运算符
    AudioEngine& operator=(const AudioEngine& other)
    {
        if (this!= &other)
        {
            m_filePath = other.m_filePath;
            m_bLoop = other.m_bLoop;
            constructFunc();
        }
        return *this;
    }

    ~AudioEngine()
    {
        PaError err = Pa_CloseStream(m_pStream);
        if (err != paNoError)
            std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        // Close the audio file
        sf_close(m_pAudioFile);
        std::lock_guard<std::mutex> lock(s_mutex);
        if (s_numInstances <= 1)
        {
            err = Pa_Terminate();
            if (err!= paNoError)
                std::cerr << "[AudioEngine::~AudioEngine] PortAudio error: " << Pa_GetErrorText(err) << std::endl;
            s_bInited = false;
        }
        s_numInstances--;
    }

    SNDFILE *getAudioFile()
    {
        return m_pAudioFile;
    }

    PaError start(bool bLoop = false)
    {
        //double  t0 = glfwGetTime() * 1000;
        if (Pa_IsStreamActive(m_pStream) || m_hasStarted)
        {
            //不用stop是因为耗时太长，会造成卡顿
            PaError err = Pa_AbortStream(m_pStream);
            if (err!= paNoError)
                std::cerr << "[AudioEngine::start] Pa_AbortStream error: " << Pa_GetErrorText(err) << std::endl;
        }
        sf_seek(m_pAudioFile, 0, SEEK_SET);
        //double  t1 = glfwGetTime() * 1000;
        m_hasStarted = true;
        m_bLoop = bLoop;
        // Start the audio stream
        PaError err = Pa_StartStream(m_pStream);
        if (err != paNoError)
            std::cerr << "[AudioEngine::start] PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        //double  t2 = glfwGetTime() * 1000;
        //std::cout << "start: " << t1 - t0 << "ms, open: " << t2 - t1 << "ms" << "total time:" << t2 - t0 << std::endl;
        return err;
    }

    PaError stop()
    {
        PaError err = Pa_StopStream(m_pStream);
        if (err != paNoError)
            std::cerr << "[AudioEngine::stop] PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        m_hasStarted = false;
        return err;
    }

    bool isActive()
    {
        return Pa_IsStreamActive(m_pStream);
    }

    static int audioCallback(const void *inputBuffer, void *outputBuffer,
                             unsigned long framesPerBuffer,
                             const PaStreamCallbackTimeInfo *timeInfo,
                             PaStreamCallbackFlags statusFlags,
                             void *userData)
    {
        AudioEngine *engine = (AudioEngine *)userData;
        SNDFILE *file = engine->getAudioFile();
        float *out = (float *)outputBuffer;
        sf_count_t numFrames = framesPerBuffer;

        // Read audio data from the file and write it to the output buffer
        sf_count_t count = sf_readf_float(file, out, numFrames);
        // Check if we've reached the end of the file
        int err = sf_error(file);
        if (err != SF_ERR_NO_ERROR)
        {
            // Otherwise, print an error message and stop the stream
            std::cerr << "[AudioEngine::audioCallback] Error reading from sound file! err: " << err << std::endl;
            return paAbort;
        }
        if (count == 0)
        {
            std::cout << "[AudioEngine::audioCallback] Playback finished" << std::endl;
            if (engine->m_bLoop)
            {
                // If we've reached the end of the file, seek back to the beginning
                sf_seek(file, 0, SEEK_SET);
                std::cout << "[AudioEngine::audioCallback] Loop Playback...Playing audio. Press Enter to stop..." << std::endl;
            }
            else
                return paComplete;
        }

        return paContinue;
    }
private:
    PaError constructFunc()
    {
        PaError err;
        if (!s_bInited)
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            // Initialize PortAudio
            err = Pa_Initialize();
            if (err != paNoError)
            {
                std::cerr << "[AudioEngine::constructFunc] PortAudio error: " << Pa_GetErrorText(err) << std::endl;
                return err;
            }
            s_bInited = true;
        }
        s_numInstances++;
        // Open the audio file using libsndfile
        SF_INFO sfInfo;
        m_pAudioFile = sf_open(m_filePath.c_str(), SFM_READ, &sfInfo);
        if (!m_pAudioFile)
        {
            std::cerr << "[AudioEngine::constructFunc] Failed to open audio file" << std::endl;
            return err;
        }

        // Set up PortAudio stream parameters
        PaStreamParameters outputParameters;
        outputParameters.device = Pa_GetDefaultOutputDevice();
        outputParameters.channelCount = sfInfo.channels;
        outputParameters.sampleFormat = paFloat32;
        outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
        outputParameters.hostApiSpecificStreamInfo = nullptr;

        // Open the PortAudio stream
        err = Pa_OpenStream(&m_pStream, nullptr, &outputParameters, sfInfo.samplerate, paFramesPerBufferUnspecified,
                            paClipOff, AudioEngine::audioCallback, this);
        if (err != paNoError)
        {
            std::cerr << "[AudioEngine::constructFunc] PortAudio error: " << Pa_GetErrorText(err) << std::endl;
            return err;
        }

        return err;
    }

private:
    static bool s_bInited;
    static int s_numInstances;
    std::string m_filePath;
    std::mutex s_mutex;
    SNDFILE * m_pAudioFile;
    bool m_bLoop;
    PaStream *m_pStream;
    bool m_hasStarted = false;
};
bool AudioEngine::s_bInited = false;
int AudioEngine::s_numInstances = 0;

