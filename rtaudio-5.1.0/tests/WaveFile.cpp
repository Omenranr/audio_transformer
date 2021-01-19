class WaveFile {
public:
    static const uint16 NUM_CHARS = 4;
public:
    WaveFile() : Data(nullptr) {}

    ~WaveFile() { delete[] Data; }

    char ChunkID[NUM_CHARS];

    uint32 ChunkSize;

    char Format[NUM_CHARS];

    char SubChunkID[NUM_CHARS];

    uint32 SubChunkSize;

    uint16 AudioFormat;

    uint16 NumChannels;

    uint32 SampleRate;

    uint32 ByteRate;

    uint16 BlockAlign;

    uint16 BitsPerSample;

    char SubChunk2ID[NUM_CHARS];

    uint32 SubChunk2Size;

    byte* Data; 
};