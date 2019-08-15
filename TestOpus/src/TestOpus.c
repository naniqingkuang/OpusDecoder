/*
 ============================================================================
 Name        : TestOpus.c
 Author      : rokid
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "opus/opus.h"

#define sample_rate		16000
#define audio_channel	1
#define opus_frame_size		320

#define file_max_len  (opus_frame_size * 100)

static OpusDecoder *decoder = NULL;

static opus_uint32 dec_final_range;
FILE *inputFile = NULL;
FILE *outFile = NULL;

int opus_init() {
	int error = 0;
	decoder = opus_decoder_create(sample_rate, audio_channel,&error);
	if(error > 0) {
		printf("decoder create error: code[%d] \n",error);
	}
}

int opus_params_init() {
	int res = opus_decoder_ctl(decoder, OPUS_GET_FINAL_RANGE(&dec_final_range));
	return res;
}

int char_to_int(char *data) {
	int destData = 0;
	int testValue = (int)data[0];
	destData = ((int)(data[0]) << 0)
			| (((int)(data[1]) << 8) & 0xFFFFFFFF)
			| (((int)(data[2]) << 16) & 0xFFFFFFFF)
			| (((int)(data[3]) << 24) & 0xFFFFFFFF);
	return destData;
}

int decoder_process(const unsigned char* tempInputData, opus_int16 *pcm, int *outLen) {
	char header[8] = {0};
	char tempData[opus_frame_size] = {0};
	memcpy(header, tempInputData, 8);
	uint32_t len = char_to_int(&header[0]);
	const uint32_t enc_final_range = char_to_int(&header[3]);

	int output_samples;
	if(len == 0)
	{
		opus_decoder_ctl(decoder, OPUS_GET_LAST_PACKET_DURATION(&output_samples));
	}
	else
	{
		output_samples = opus_frame_size;
	}
	if(len > 0 && len <= opus_frame_size) {
		memset(tempData, 0, opus_frame_size);
		memcpy(tempData, &tempInputData[8],len);
		output_samples = opus_decode(decoder,tempData, len, pcm, output_samples, 0);
		if(output_samples <= 0) {
			printf("error decode exit!\n");
			exit(0);
		}
		*outLen = output_samples;
		uint32_t dec_final_range;
		opus_decoder_ctl(decoder, OPUS_GET_FINAL_RANGE(&dec_final_range));
		return len + 8;
	} else {
		printf("error or finish decode exit!\n");
		exit(0);
	}


}



int main(void) {
	opus_init();
	opus_params_init();

	//file init
	inputFile = fopen("./test.opu", "r");
	if(inputFile < 0) {
		printf("input opu file error!");
	}

	outFile = fopen("./output.pcm", "w");
	if(outFile < 0) {
		printf("output file create error!\n");
	}

	size_t inputLen = 0;
	fseek(inputFile, 0 , SEEK_END);
	int fileLen = ftell(inputFile);
	fseek(inputFile, 0 , SEEK_SET);

	if(fileLen < 8) {
		printf("file is too small!\n");
	}
	unsigned char *inputData = (unsigned char *) malloc(fileLen);
	memset(inputData,0,fileLen);


	inputLen = fread((void*)inputData, 1 , fileLen, inputFile);
	if(inputLen < 0) {
		printf("err in read opufile!\n");
	}

	int opuIndex = 0;
	while(opuIndex < inputLen - 8) {
		int outLen = 0;
		opus_int16 outData[opus_frame_size] = {0};
		opuIndex += decoder_process(&inputData[opuIndex], &outData, &outLen);
		if(outLen > 8) {
			fwrite(outData, sizeof(opus_int16), outLen, outFile);
		} else {
			break;
		}
	}
	printf("opus decode success!\n");
	return EXIT_SUCCESS;
}
