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
        PaError err;
        if (!s_bInited)
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            // Initialize PortAudio
            err = Pa_Initialize();
            if (err != paNoError)
            {
                std::cerr << "[AudioEngine::AudioEngine] PortAudio error: " << Pa_GetErrorText(err) << std::endl;
                return;
            }
            s_bInited = true;
            s_numInstances++;
        }

        // Open the audio file using libsndfile
        SF_INFO sfInfo;
        m_pAudioFile = sf_open(filePath, SFM_READ, &sfInfo);
        if (!m_pAudioFile)
        {
            std::cerr << "[AudioEngine::AudioEngine] Failed to open audio file" << std::endl;
            return;
        }

        // Set up PortAudio stream parameters
        PaStreamParameters outputParameters;
        outputParameters.device = Pa_GetDefaultOutputDevice();
        outputParameters.channelCount = sfInfo.channels;
        outputParameters.sampleFormat = paFloat32;
        outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
        outputParameters.hostApiSpecificStreamInfo = nullptr;

        // Open the PortAudio stream
        err = Pa_OpenStream(&m_stream, nullptr, &outputParameters, sfInfo.samplerate, paFramesPerBufferUnspecified,
                            paClipOff, AudioEngine::audioCallback, this);
        if (err != paNoError)
        {
            std::cerr << "[AudioEngine::AudioEngine] PortAudio error: " << Pa_GetErrorText(err) << std::endl;
            return;
        }
    }

    ~AudioEngine()
    {
        PaError err = Pa_CloseStream(m_stream);
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
        if (Pa_IsStreamActive(m_stream) || m_hasStarted)
            stop();
        sf_seek(m_pAudioFile, 0, SEEK_SET);
        m_hasStarted = true;
        m_bLoop = bLoop;
        // Start the audio stream
        PaError err = Pa_StartStream(m_stream);
        if (err != paNoError)
            std::cerr << "[AudioEngine::start] PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return err;
    }

    PaError stop()
    {
        PaError err = Pa_StopStream(m_stream);
        if (err != paNoError)
            std::cerr << "[AudioEngine::stop] PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        m_hasStarted = false;
        return err;
    }

    bool isActive()
    {
        return Pa_IsStreamActive(m_stream);
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
    static bool s_bInited;
    static int s_numInstances;
    std::mutex s_mutex;
    SNDFILE * m_pAudioFile;
    bool m_bLoop;
    PaStream *m_stream;
    bool m_hasStarted = false;
};
bool AudioEngine::s_bInited = false;
int AudioEngine::s_numInstances = 0;

