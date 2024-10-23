#include "WAVController.hpp"

#include <Windows.h>
#include <mmreg.h>
#include <cstdint>
#include <print>

extern const uint32_t SAMPLE_RATE;
extern const uint32_t DURATION;

static constexpr uint16_t cBitsPerSample = 32;
static constexpr uint16_t cChannels = 1;

#pragma pack(push, 1)
struct WAVHeader {
    // RIFF Header
    char riff[4] = { 'R', 'I', 'F', 'F' };
    uint32_t fileSize;    // Size of the entire file minus 8 bytes
    char wave[4] = { 'W', 'A', 'V', 'E' };

    // Format Chunk
    char fmtChunkID[4] = { 'f', 'm', 't', ' ' };
    uint32_t fmtChunkSize = 16;  // PCM format
    uint16_t audioFormat = 3;    // IEEE Float = 3
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;       // sampleRate * numChannels * bitsPerSample / 8
    uint16_t blockAlign;     // numChannels * bitsPerSample / 8
    uint16_t bitsPerSample;

    // Data Chunk
    char dataChunkID[4] = { 'd', 'a', 't', 'a' };
    uint32_t dataChunkSize;  // Size of the audio data
};
#pragma pack(pop)

void WAVController::PlaylayWAV(const std::vector<float>& data)
{
    // Set up the waveform audio format
    WAVEFORMATEX wfx;
    wfx.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    wfx.nChannels = cChannels;  // Mono
    wfx.nSamplesPerSec = SAMPLE_RATE;
    wfx.wBitsPerSample = cBitsPerSample;  // 32-bit float audio
    wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;


    WAVEHDR whdr;
    whdr.lpData = (LPSTR)data.data();
    whdr.dwBufferLength = data.size() * sizeof(float);
    whdr.dwFlags = 0;
    whdr.dwLoops = 0;

    HWAVEOUT hWaveOut;
    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) 
    {
        std::print(stderr, "Failed to open wave output device.\n");
    }

    waveOutPrepareHeader(hWaveOut, &whdr, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &whdr, sizeof(WAVEHDR));

    Sleep(DURATION * 1000);

    waveOutUnprepareHeader(hWaveOut, &whdr, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
}

void WAVController::CreateWAVFile(std::string &&name, const std::vector<float> &data)
{
    // Create and initialize the WAV header
    WAVHeader header;
    header.numChannels = cChannels;
    header.sampleRate = SAMPLE_RATE;
    header.bitsPerSample = cBitsPerSample;
    header.blockAlign = header.numChannels * header.bitsPerSample / 8;
    header.byteRate = SAMPLE_RATE * header.blockAlign;
    header.dataChunkSize = data.size() * sizeof(float);
    header.fileSize = sizeof(WAVHeader) + header.dataChunkSize;

    // Create the WAV file using WinAPI
    HANDLE hFile = CreateFileA(
        name.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        std::print(stderr, "Failed to create file: {}", name);
        return;
    }

    // Write WAV header
    DWORD bytesWritten;
    WriteFile(hFile, &header, sizeof(header), &bytesWritten, nullptr);

    // Write audio data (IEEE Float data)
    WriteFile(hFile, data.data(), header.dataChunkSize, &bytesWritten, nullptr);
    int err = GetLastError();
    // Close the file
    CloseHandle(hFile);
    std::print("WAV file created: {}", name);
}


