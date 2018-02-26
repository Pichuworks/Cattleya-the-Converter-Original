/*
    Classic AMR2WAV Converter
    Codename Cattleya
    Developed by Pichu
    Jan. 18th, '19

    ClassAMR2WAV C File

    ref: 
    http://blog.csdn.net/dinggo/article/details/2002298
    https://www.cnblogs.com/wangguchangqing/p/5957531.html

*/ 

#include "ClassicAMRCodec.h"
#include "ClassicAMRCodecDPL.h"
#include "ClassicAMRCodecDPP.h"
/**
 * д WAV �ļ�ͷ
 * ������ 8KHz 16bit 
 * @param   wav�ļ���int֡
 * @return  void
 */
void WAVFileHeaderWriter(FILE *file_wav, int Frame) {
	RIFFChunk riff; 
	ChunkHeader chunk;
	WAVExtBlock wav_fmt_ext;
	
    char tag[10] = "";

    // 1. д RIFF chunk ͷ
    strcpy(tag, "RIFF");
    memcpy(riff.riff_ID, tag, 4);                       // д RIFF chunk
    riff.riff_size = 4                                  // WAV
                     + sizeof(ChunkHeader)              // fmt
                     + sizeof(WAVExtBlock)              // WAVEFORMATX
                     + sizeof(ChunkHeader)              // DATA
                     + Frame * 160 * sizeof(short);
    strcpy(tag, "WAVE");
    memcpy(riff.riff_format, tag, 4);
    fwrite(&riff, 1, sizeof(RIFFChunk), file_wav);

    // 2. д Format chunk
    strcpy(tag, "fmt ");
    memcpy(chunk.chunk_ID, tag, 4);
    chunk.chunk_size = sizeof(WAVExtBlock);
    fwrite(&chunk, 1, sizeof(ChunkHeader), file_wav);
    memset(&wav_fmt_ext, 0, sizeof(WAVExtBlock));
    wav_fmt_ext.format_tag = 1;
    wav_fmt_ext.channels = 1;         			// ������
    wav_fmt_ext.samples_per_sec = 8000; 		// 8KHz 
    wav_fmt_ext.avg_bytes_per_sec = 16000;
    wav_fmt_ext.block_align = 2;
    wav_fmt_ext.bits_per_sample = 16; 			// 16λ
    fwrite(&wav_fmt_ext, 1, sizeof(WAVExtBlock), file_wav);

    // 3. д data chunk ͷ
    strcpy(tag, "data");
    memcpy(chunk.chunk_ID, tag, 4);
    chunk.chunk_size = Frame * 160 * sizeof(short);
    fwrite(&chunk, 1, sizeof(ChunkHeader), file_wav);
}

/**
 * ���㵱ǰ֡��С
 * @param   frame_header
 * @return  frame_size
 */
int calcFrameSize(unsigned char frame_header) {
    int mode, frame_size;
    int tmp = 0;

    // ���뷽ʽ��� = ֡ͷ��3-6λ
    frame_header &= 0x78; // 0111-1000
    frame_header >>= 3;
 
    mode = amrEncodeMode[frame_header];
 
    // ����amr��Ƶ����֡��С
    // ԭ��: amrһ֡��Ӧ20ms����ôһ����50֡����Ƶ����
    tmp = (double)(((double) mode / (double) AMR_FRAME_COUNT_PER_SECOND) / (double) 8);
    tmp = ((int)(tmp + 0.5));
    frame_size = ((int)((((double)tmp + 0.5) + 0.5)));
    return frame_size;
}

/**
 * ����֡��Ϊ�ο�֡
 * @param   amr�ļ�����Ӧ��buffer��֡��С��֡ͷ
 * @return  0 for FALSE, 1 for TRUE
 */
int readFirstFrame(FILE *file_amr, unsigned char frame_buffer[], int *std_frame_size, unsigned char *std_frame_header) {
    // Init
    memset(frame_buffer, 0, sizeof(frame_buffer));
 
    // ��֡ͷ
    fread(std_frame_header, 1, sizeof(unsigned char), file_amr);
    if (feof(file_amr)) return 0;
 
    // ����֡ͷ����֡��С
    *std_frame_size = calcFrameSize(*std_frame_header);
 
    // ����֡����
    frame_buffer[0] = *std_frame_header;
    fread(&(frame_buffer[1]), 1, (*std_frame_size-1)*sizeof(unsigned char), file_amr);
    if (feof(file_amr)) return 0;
 
    return 1;
}

/**
 * ��AMR֡
 * @param   amr�ļ�����Ӧ��buffer��֡��С��֡ͷ
 * @return  0 for FALSE, 1 for TRUE
 */
int readOrdinaryFrame(FILE *file_amr, unsigned char frame_buffer[], int std_frame_size, unsigned char std_frame_header) {
    int bytes = 0;
    unsigned char frame_header;
 
    memset(frame_buffer, 0, sizeof(frame_buffer));
 
    // ���׼֡ͷ�Աȣ���Ϊ��֡�����������һ���ֽ���������׼֡ͷ
    while(1) {
        bytes = fread(&frame_header, 1, sizeof(unsigned char), file_amr);
        if (feof(file_amr)) return 0;
        if (frame_header == std_frame_header) break;
    }
 
    // ����֡����������
    frame_buffer[0] = frame_header;
    bytes = fread(&(frame_buffer[1]), 1, (std_frame_size-1)*sizeof(unsigned char), file_amr);
    if (feof(file_amr)) return 0;
 
    return 1;
}

/**
 * ������AMRת����WAV_void
 * @param   AMR�ļ�����WAV�ļ���
 * @return  ֡����_int null_void
 */
void classicAMR2WAV(const char *AMR_filename, const char *WAV_filename) {
    FILE *amr_file = NULL;
    FILE *wav_file = NULL;
    char magic[8];
    int *destate;           // ?
    int frame_counter = 0;
    int std_frame_size;
    unsigned char std_frame_header;
 
    unsigned char amr_frame[MAX_AMR_FRAME_SIZE];
    short wav_frame[WAV_FRAME_SIZE];
 
    amr_file = fopen(AMR_filename, "rb");
    if(amr_file==NULL) return 0;
 
    // ���amr�ļ�ͷ
    fread(magic, sizeof(char), strlen(AMR_MAGIC_NUMBER), amr_file);
    if(strncmp(magic, AMR_MAGIC_NUMBER, strlen(AMR_MAGIC_NUMBER))) {
        fclose(amr_file);
        return 0;
    }
 
    // ��������ʼ��WAV�ļ�
    wav_file = fopen(WAV_filename, "wb");
    WAVFileHeaderWriter(wav_file, frame_counter);
 
    // ��ʼ��������
    destate = Decoder_Interface_init();             // ?
 
    // ����֡��Ϊ�ο�֡
    memset(amr_frame, 0, sizeof(amr_frame));
    memset(wav_frame, 0, sizeof(wav_frame));
    readFirstFrame(amr_file, amr_frame, &std_frame_size, &std_frame_header);
 
    // ����һ��AMR��Ƶ֡��WAV����
    Decoder_Interface_Decode(destate, amr_frame, wav_frame, 0);
    frame_counter++;
    fwrite(wav_frame, sizeof(short), WAV_FRAME_SIZE, wav_file);
 
    // ��֡����AMR��д��WAV
    while(1) {
        memset(amr_frame, 0, sizeof(amr_frame));
        memset(wav_frame, 0, sizeof(wav_frame));
        if (!readOrdinaryFrame(amr_file, amr_frame, std_frame_size, std_frame_header))
            break;
        // ����һ��AMR��Ƶ֡��WAV���ݣ�����Ϊ 8k 16bit ������
        Decoder_Interface_Decode(destate, amr_frame, wav_frame, 0);
        frame_counter++;
        fwrite(wav_frame, sizeof(short), WAV_FRAME_SIZE, wav_file);
    }
 
    Decoder_Interface_exit(destate);    // ?
 
    fclose(wav_file);
 
    // ��дWAV�ļ�ͷ
    wav_file = fopen(WAV_filename, "r+");
    WAVFileHeaderWriter(wav_file, frame_counter);
    fclose(wav_file);
 
    // return frame_counter;
    return;
}

/* 
int main() {
    const char AMR_filename[] = "test.amr";
    const char WAV_filename[] = "test.wav";
    classicAMR2WAV(AMR_filename, WAV_filename);
    return 0;
}
*/
int main(int argc, char *argv[]) {
	if(argc != 3) {
		printf("\nClassic AMR2WAV Converter for Windows\n");
		printf("Codename Cattleya\n");
		printf("Developed by Pichu\n");
		printf("Version 1.2.0001 Compile date: Feb 26th, '18\n\n");
		printf("Syntax: ClassicAMR2WAV.exe filename.amr filename.wav\n\n");
		
	}
	else {
		const char *AMR_filename = argv[1];
		const char *WAV_filename = argv[2];
		classicAMR2WAV(AMR_filename, WAV_filename);	
	}
	return 0;
}
