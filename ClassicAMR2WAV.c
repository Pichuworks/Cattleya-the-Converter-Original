/*
    Classic AMR2WAV Converter
    Codename Cattleya
    Developed by Pichu
    Jan. 18th, '19

    ref: 
    http://blog.csdn.net/dinggo/article/details/2002298
    https://www.cnblogs.com/wangguchangqing/p/5957531.html

*/ 

#include "ClassicAMRCodec.h"

// 写 WAV 文件头
void WAVFileHeaderWriter(FILE *file_wav, int Frame) {
    char tag[10] = "";

    // 1. 写 RIFF chunk 头
    strcpy(tag, "RIFF");
    memcpy(riff.riff_ID, tag, 4);                       // 写 RIFF chunk
    riff.riff_size = 4                                  // WAV
                     + sizeof(ChunkHeader)              // fmt
                     + sizeof(WAVExtBlock)              // WAVEFORMATX
                     + sizeof(ChunkHeader)              // DATA
                     + Frame * 160 * sizeof(short);
    strcpy(tag, "WAVE");
    memcpy(riff.riff_format, tag, 4);
    fwrite(&riff, 1, sizeof(RIFFChunk), file_wav);

    // 2. 写 Format chunk
    strcpy(tag, "fmt ");
    memcpy(chunk.chunk_ID, tag, 4);
    chunk.chunk_size = sizeof(WAVExtBlock);
    fwrite(&chunk, 1, sizeof(ChunkHeader), file_wav);
    memset(&wav_fmt_ext, 0, sizeof(WAVExtBlock));
    wav_fmt_ext.format_tag = 1;
    wav_fmt_ext.channels = 1;         // 单声道
    wav_fmt_ext.samples_per_sec = 8000; // 8khz
    wav_fmt_ext.avg_bytes_per_sec = 16000;
    wav_fmt_ext.block_align = 2;
    wav_fmt_ext.bits_per_sample = 16; // 16位
    fwrite(&wav_fmt_ext, 1, sizeof(WAVExtBlock), file_wav);

    // 3. 写 data chunk 头
    strcpy(tag, "data");
    memcpy(chunk.chunk_ID, tag, 4);
    chunk.chunk_size = Frame * 160 * sizeof(short);
    fwrite(&chunk, 1, sizeof(ChunkHeader), file_wav);
}

const int round(const double x) {               // ?
    return((int)(x+0.5));
}

// 根据帧头计算当前帧大小
int calcFrameSize(unsigned char frame_header) {
    int mode, frame_size;
    int tmp = 0;

    // 编码方式编号 = 帧头的3-6位
    frame_header &= 0x78; // 0111-1000
    frame_header >>= 3;
 
    mode = amrEncodeMode[frame_header];
 
    // 计算amr音频数据帧大小
    // 原理: amr 一帧对应20ms，那么一秒有50帧的音频数据
    tmp = (double)(((double) mode / (double) AMR_FRAME_COUNT_PER_SECOND) / (double) 8);
    tmp = round(tmp);
    frame_size = round((double)tmp + 0.5);
    return frame_size;
}