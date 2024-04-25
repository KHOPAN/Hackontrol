#include "errorLog.h"
#include "sha512.h"

const unsigned long long constantSmall[8] = {0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL, 0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL, 0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL, 0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL};
const unsigned long long constantBig[80] = {0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL, 0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL, 0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL, 0xd807aa98a3030242ULL, 0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL, 0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL, 0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL, 0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL, 0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL, 0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL, 0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL, 0x06ca6351e003826fULL, 0x142929670a0e6e70ULL, 0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL, 0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL, 0x92722c851482353bULL, 0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL, 0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL, 0xd192e819d6ef5218ULL, 0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL, 0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL, 0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL, 0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL, 0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL, 0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL, 0xca273eceea26619cULL, 0xd186b8c721c0c207ULL, 0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL, 0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL, 0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL, 0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL, 0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL};

char* sha512Hash(BYTE* data, size_t dataLength) {
	size_t inputBits = dataLength * 8;
	size_t paddingSize = (895 - inputBits) % 1024;
	size_t bufferSize = (inputBits + paddingSize + 129) / 1024;
	unsigned long long** buffer = malloc(sizeof(unsigned long long) * bufferSize);

	if(!buffer) {
		dialogError(ERROR_OUTOFMEMORY, L"malloc");
		return NULL;
	}

	for(size_t i = 0; i < bufferSize; i++) {
		buffer[i] = malloc(sizeof(unsigned long long) * 16);

		if(!buffer[i]) {
			dialogError(ERROR_OUTOFMEMORY, L"malloc");
			return NULL;
		}
	}

	for(size_t x = 0; x < bufferSize; x++) {
		for(size_t y = 0; y < 16; y++) {
			unsigned long long bit = 0x0ULL;

			for(size_t z = 0; z < 8; z++) {
				size_t index = x * 128 + y * 8 + z;

				if(index < dataLength) {
					bit = bit << 8 | (unsigned long long) data[index];
				} else if(index == dataLength) {
					bit = bit << 8 | 0x80ULL;
				} else {
					bit = bit << 8 | 0x0ULL;
				}
			}

			buffer[x][y] = bit;
		}
	}

	buffer[bufferSize - 1][16 - 1] = inputBits;
	buffer[bufferSize - 1][16 - 2] = 0x00ULL;
	unsigned long long* holder = malloc(sizeof(unsigned long long) * 8);

	if(!holder) {
		dialogError(ERROR_OUTOFMEMORY, L"malloc");
		return NULL;
	}

	unsigned long long smallHolder[8];
	unsigned long long bigHolder[80];
	memcpy(holder, constantSmall, sizeof(unsigned long long) * 8);

	for(size_t x = 0; x < bufferSize; x++) {
		memcpy(bigHolder, buffer[x], sizeof(unsigned long long) * 16);
		memcpy(smallHolder, holder, sizeof(unsigned long long) * 8);

		for(size_t y = 16; y < 80; y++) {
			bigHolder[y] = bigHolder[y - 16] + (((bigHolder[y - 15] >> 1) | (bigHolder[y - 15] << ((sizeof(bigHolder[y - 15]) << 3) - 1))) ^ ((bigHolder[y - 15] >> 8) | (bigHolder[y - 15] << ((sizeof(bigHolder[y - 15]) << 3) - 8))) ^ (bigHolder[y - 15] >> 7)) + bigHolder[y - 7] + (((bigHolder[y - 2] >> 19) | (bigHolder[y - 2] << ((sizeof(bigHolder[y - 2]) << 3) - 19))) ^ ((bigHolder[y - 2] >> 61) | (bigHolder[y - 2] << ((sizeof(bigHolder[y - 2]) << 3) - 61))) ^ (bigHolder[y - 2] >> 6));
		}

		for(size_t y = 0; y < 80; y++) {
			unsigned long long z = smallHolder[7] + ((((smallHolder[4] >> 14) | (smallHolder[4] << ((sizeof(smallHolder[4]) << 3) - 14)))) ^ (((smallHolder[4] >> 18) | (smallHolder[4] << ((sizeof(smallHolder[4]) << 3) - 18)))) ^ (((smallHolder[4] >> 41) | (smallHolder[4] << ((sizeof(smallHolder[4]) << 3) - 41))))) + ((smallHolder[4] & smallHolder[5]) ^ (~smallHolder[4] & smallHolder[6])) + constantBig[y] + bigHolder[y];
			unsigned long long a = ((((smallHolder[0] >> 28) | (smallHolder[0] << ((sizeof(smallHolder[0]) << 3) - 28)))) ^ (((smallHolder[0] >> 34) | (smallHolder[0] << ((sizeof(smallHolder[0]) << 3) - 34)))) ^ (((smallHolder[0] >> 39) | (smallHolder[0] << ((sizeof(smallHolder[0]) << 3) - 39))))) + ((smallHolder[0] & smallHolder[1]) ^ (smallHolder[0] & smallHolder[2]) ^ (smallHolder[1] & smallHolder[2]));
			smallHolder[7] = smallHolder[6];
			smallHolder[6] = smallHolder[5];
			smallHolder[5] = smallHolder[4];
			smallHolder[4] = smallHolder[3] + z;
			smallHolder[3] = smallHolder[2];
			smallHolder[2] = smallHolder[1];
			smallHolder[1] = smallHolder[0];
			smallHolder[0] = z + a;
		}

		for(size_t y = 0; y < 8; y++) {
			holder[y] += smallHolder[y];
		}
	}

	for(size_t i = 0; i < bufferSize; i++) {
		free(buffer[i]);
	}

	free(buffer);
	char* output = malloc(129);

	if(!output) {
		dialogError(ERROR_OUTOFMEMORY, L"malloc");
		return NULL;
	}

	for(int i = 0; i < 8; i++) {
		sprintf_s(output + i * 16, 17, "%016I64x", holder[i]);
	}

	free(holder);
	return output;
}
