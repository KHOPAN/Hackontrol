package com.khopan.hrsp;

import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;

public class QOIDecoder {
	public QOIDecoder() {

	}

	public BufferedImage decode(int width, int height, byte[] data) {
		ByteArrayInputStream stream = new ByteArrayInputStream(data);
		BufferedImage image = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);
		int[] pixels = new int[width * height];
		int[] indexTable = new int[64];
		int red = 0;
		int green = 0;
		int blue = 0;

		for(int i = 0; i < pixels.length; i++) {
			int chunk = stream.read() & 0xFF;

			if(chunk == 0b11111110) {
				red = stream.read();
				green = stream.read();
				blue = stream.read();
			} else {
				switch(chunk & 0b11000000) {
				case 0b00000000:
					int index = chunk & 0b111111;
					red = (indexTable[index] >> 16) & 0xFF;
					green = (indexTable[index] >> 8) & 0xFF;
					blue = indexTable[index] & 0xFF;
					break;
				case 0b01000000:
					red += ((chunk >> 4) & 0b11) - 2;
					green += ((chunk >> 2) & 0b11) - 2;
					blue += (chunk & 0b11) - 2;
					break;
				case 0b10000000:
					int next = stream.read();
					int differenceGreen = (chunk & 0b111111) - 32;
					red += differenceGreen - 8 + ((next >> 4) & 0b1111);
					green += differenceGreen;
					blue += differenceGreen - 8 + (next & 0b1111);
					break;
				case 0b11000000:
					for(int z = 0; z < (chunk & 0b111111); z++) {
						pixels[i] = ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF);
						i++;
					}

					break;
				}
			}

			int indexPosition = ((red & 0xFF) * 3 + (green & 0xFF) * 5 + (blue & 0xFF) * 7 + 0xFF * 11) & 0b111111;
			indexTable[indexPosition] = ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF);
			pixels[i] = indexTable[indexPosition];
		}

		image.getRaster().setDataElements(0, 0, width, height, pixels);
		return image;
	}
}
