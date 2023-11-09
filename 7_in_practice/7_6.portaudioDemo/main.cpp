// 使用PortAudio和SoundTouch播放音频文件的示例代码，已测试通过wav和mp3格式
#include <iostream>
#include <portaudio.h>
#include <sndfile.h>

#define FILE_PATH RESOURCES_DIR "/audio/powerup.wav" // powerup.wav breakout.mp3

bool g_bLoop = true;
bool g_bKeyPressed = false;

// Callback function to fill the audio buffer
int audioCallback(const void *inputBuffer, void *outputBuffer,
                  unsigned long framesPerBuffer,
                  const PaStreamCallbackTimeInfo *timeInfo,
                  PaStreamCallbackFlags statusFlags,
                  void *userData)
{
    SNDFILE *file = (SNDFILE *)userData;
    float *out = (float *)outputBuffer;
    sf_count_t numFrames = framesPerBuffer;

    // Read audio data from the file and write it to the output buffer
    int count = sf_readf_float(file, out, numFrames);
    // Check if we've reached the end of the file
    int err = sf_error(file);
    if (err != SF_ERR_NO_ERROR)
    {
        // Otherwise, print an error message and stop the stream
        std::cerr << "Error reading from sound file! err: " << err << std::endl;
        return paAbort;
    }
    if (count == 0)
    {
        std::cout << "Playback finished" << std::endl;
        if (g_bLoop)
        {
            // If we've reached the end of the file, seek back to the beginning
            sf_seek(file, 0, SEEK_SET);
            std::cout << "Loop Playback...Playing audio. Press Enter to stop..." << std::endl;
        }
        else
            return paComplete;
    }

    return paContinue;
}

int main()
{
    // Initialize PortAudio
    PaError err;
    err = Pa_Initialize();
    if (err != paNoError)
    {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    // Open the audio file using libsndfile
    SF_INFO sfInfo;
    SNDFILE *audioFile = sf_open(FILE_PATH, SFM_READ, &sfInfo);
    if (!audioFile)
    {
        std::cerr << "Failed to open audio file" << std::endl;
        return 1;
    }

    // Set up PortAudio stream parameters
    PaStream *stream;
    PaStreamParameters outputParameters;
    outputParameters.device = Pa_GetDefaultOutputDevice();
    outputParameters.channelCount = sfInfo.channels;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = nullptr;

    // Open the PortAudio stream
    err = Pa_OpenStream(&stream, nullptr, &outputParameters, sfInfo.samplerate,
                        paFramesPerBufferUnspecified, paClipOff, audioCallback, audioFile);
    if (err != paNoError)
    {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    // Start the audio stream
    err = Pa_StartStream(stream);
    if (err != paNoError)
    {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    std::cout << "Playing audio. Press Enter to stop..." << std::endl;
    std::cin.get();
    // while (Pa_IsStreamActive(stream) && g_bKeyPressed == false)
    // {
    //     Pa_Sleep(100);
    // }

    // Stop and close the stream
    err = Pa_StopStream(stream);
    if (err != paNoError)
    {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
    }
    err = Pa_CloseStream(stream);
    if (err != paNoError)
    {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
    }

    // Close the audio file
    sf_close(audioFile);

    // Terminate PortAudio
    Pa_Terminate();

    return 0;
}
