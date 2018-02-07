/*
    Classic AMR2WAV Converter
    Codename Cattleya
    Developed by Pichu
    Jan. 18th, '19

    ClassAMR2WAV Decode Header File

    ref: 
    http://blog.csdn.net/dinggo/article/details/2002298
    https://www.cnblogs.com/wangguchangqing/p/5957531.html

*/ 

#ifndef CLASSIC_AMR_CODEC_H
#define CLASSIC_AMR_CODEC_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interf_rom.h"

#define AMR_MAGIC_NUMBER "#!AMR\n"
#define WAV_FRAME_SIZE 160
#define MAX_AMR_FRAME_SIZE 32
#define AMR_FRAME_COUNT_PER_SECOND 50

/*
    WAVʹ�õ�RIFF CHUNK����
    ref: https://www.cnblogs.com/wangguchangqing/p/5957531.html
    struct chunk {
        uint32_t id;   // ���־
        uint32_t size; // ���С
        uint8_t data[size]; // ������
    };
    һ��WAV�ļ���������һ��RIFF chunk��RIFF chunk�ְ�����Format chunk��Data chunk�Լ���ѡ��Fact chunk��

    RIFF chunk
    id��FOURCC ֵΪ'R' 'I' 'F' 'F'
    size����data�ֶ������ݵĴ�С �ֽ���
    data������������chunk

    Format chunk
    id��FOURCC ֵΪ 'f' 'm' 't' ' '
    size�������ֶΰ������ݵĴ�С��������չ�飬��ֵΪ16������չ�飬��ֵΪ= 16 + 2�ֽ���չ�鳤�� + ��չ�鳤�Ȼ���ֵΪ18��ֻ����չ��ĳ���Ϊ2�ֽڣ�ֵΪ0��
    data�������Ƶ��ʽ���������������ʵ���Ϣ
    format_tag��2�ֽڣ���ʾ��Ƶ���ݵĸ�ʽ����ֵΪ1����ʾʹ��PCM��ʽ��
    channels��2�ֽڣ���������ֵΪ1��Ϊ��������Ϊ2����˫������
    samples_per_sec�������ʣ���Ҫ��22.05KHz��44.1kHz��48KHz��
    avg_bytes_per_sec����Ƶ�����ʣ�ÿ�벥�ŵ��ֽ�����samples_per_sec * channels * bits_per_sample / 8�����Թ����ʹ�û������Ĵ�С
    block_align�����ݿ���뵥λ��һ�β����Ĵ�С��ֵΪ������ * ����λ�� / 8���ڲ���ʱ��Ҫһ�δ�������ֵ��С���ֽ����ݡ�
    bits_per_sample����Ƶsample������λ������16λ��24λ��32λ�ȡ�
    cbSize����չ���ĳ���
    ��չ�����ݣ�22�ֽڣ�������ܣ����油�䡣
*/

typedef struct {
    char chunk_ID[4];
    int chunk_size;
} ChunkHeader;  // XCHUNKHEADER

typedef struct {
    char riff_ID[4];
    int riff_size;
    char riff_format[4];
} RIFFChunk;    // RIFFHEADER

typedef struct {
    short format_tag;
    short channels;
    int samples_per_sec;
    int avg_bytes_per_sec;
    short block_align;
    short bits_per_sample;
} WAVBlock; // WAVEFORMAT

typedef struct {
    short format_tag;
    short channels;
    int samples_per_sec;
    int avg_bytes_per_sec;
    short block_align;
    short bits_per_sample;
    short ext_size;
} WAVExtBlock; // WAVEFORMATX

typedef struct {
    char format_ID[4];
    int format_size;
    WAVBlock wave;
} WAVChunk; // FMTBLOCK

int amrEncodeMode[] = {4750, 5150, 5900, 6700, 7400, 7950, 10200, 12200}; // amr���뷽ʽ

void classicAMR2WAV(const char *AMR_file_name, const char *WAV_file_name);
#endif
