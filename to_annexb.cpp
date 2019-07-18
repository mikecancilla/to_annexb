/*
    Original code by Mike Cancilla (https://github.com/mikecancilla)
    2019

    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any
    damages arising from the use of this software.

    Permission is granted to anyone to use this software for any
    purpose, including commercial applications, and to alter it and
    redistribute it freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must
    not claim that you wrote the original software. If you use this
    software in a product, an acknowledgment in the product documentation
    would be appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and
    must not be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

/*
    This is a Windows console application.

    This program will convert and H264 or H265 elementary stream into the AnnexB format.  The AnnexB format is good for streaming.
*/

#include <Shlwapi.h>
#include <cstdint>
#include <cstdlib>
#include <cstdio>

#define BUFFER_SIZE (1024*1024)

inline long SwapLong(const uint8_t* pByte)
{
    return (pByte[0] << 24) |
            (pByte[1] << 16) |
            (pByte[2] << 8)  |
            pByte[3];
}

int main(int argc, char* argv[])
{
	PathRemoveExtensionA(argv[0]);
	char *programName = PathFindFileName(argv[0]);

	if (1 == argc)
    {
        fprintf(stderr, "Usage: %s file_to_convert\n", programName);
        return 0;
    }

	char pFileName[MAX_PATH];
	memcpy(pFileName, argv[1], strlen(argv[1])+1);

    char *pExtension = PathFindExtensionA(pFileName);
    char pExtChars[12];
    strcpy(pExtChars, pExtension);

	PathRemoveExtensionA(pFileName);
	strcat(pFileName, "_annexb");
	strcat(pFileName, pExtChars);

	FILE *fOut = fopen(pFileName, "wb");
	if (NULL == fOut)
    {
        fprintf(stderr, "%s: Can't open output file", pFileName);
		return -1;
    }

	FILE *fIn = fopen(argv[1], "rb");
	if (NULL == fIn)
    {
        fprintf(stderr, "%s: Can't open input file", argv[0]);
		return -1;
    }

    // Determine the size of the file
    fseek(fIn, 0L, SEEK_END);
    size_t fileSize = ftell(fIn);
    fseek(fIn, 0L, SEEK_SET);

	uint32_t annexbCode = 0x01000000;
	long curPos = 0;
	byte *b = (byte *) malloc(1024 * 1024);

	while (curPos < fileSize)
	{
		uint8_t temp_buffer[4];
		fread(temp_buffer, 1, 4, fIn);
		curPos +=4;

		uint32_t naluSize = SwapLong(temp_buffer);
		fwrite(&annexbCode, 4, 1, fOut);

		uint32_t readTotal = naluSize;
		uint32_t readSize = min(BUFFER_SIZE, readTotal);

		while(readSize)
		{
			fread(b, readSize, 1, fIn);
			fwrite(b, readSize, 1, fOut);

			readTotal -= readSize;
			readSize = min(BUFFER_SIZE, readTotal);
		}

		double percentDone = (double) curPos / fileSize;
		printf("%s: %d%%\r", programName, (uint32_t)(percentDone * 100.));
		curPos += naluSize;
	}

	printf("%s: 100%%\n", programName);

	fclose(fIn);
	fclose(fOut);
	free(b);

    return 0;
}

