package com.khopan.hrsp;

import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;

public class QOIDecoder {
	public static final int QOI_OP_RGB   = 0b11111110;
	public static final int QOI_OP_INDEX = 0b00000000;
	public static final int QOI_OP_DIFF  = 0b01000000;
	public static final int QOI_OP_LUMA  = 0b10000000;
	public static final int QOI_OP_RUN   = 0b11000000;
	public static final int OP_MASK      = 0b11000000;

	private int width;
	private int height;

	public void size(int width, int height) {
		this.width = width;
		this.height = height;
	}

	public BufferedImage decode(byte[] data) {
		if(this.width * this.height < 1) {
			throw new IllegalArgumentException("Size is not set");
		}

		ByteArrayInputStream stream = new ByteArrayInputStream(data);
		BufferedImage image = new BufferedImage(this.width, this.height, BufferedImage.TYPE_INT_RGB);
		int[] pixels = new int[this.width * this.height];
		int[] indexTable = new int[64];
		int red = 0;
		int green = 0;
		int blue = 0;

		for(int i = 0; i < pixels.length; i++) {
			int chunk = stream.read() & 0xFF;

			if(chunk == QOIDecoder.QOI_OP_RGB) {
				red = stream.read();
				green = stream.read();
				blue = stream.read();
				pixels[i] = indexTable[((red & 0xFF) * 3 + (green & 0xFF) * 5 + (blue & 0xFF) * 7 + 0xFF * 11) & 0b111111] = ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF);
				continue;
			}

			switch(chunk & QOIDecoder.OP_MASK) {
			case QOIDecoder.QOI_OP_INDEX:
				int index = chunk & 0b111111;
				red = (indexTable[index] >> 16) & 0xFF;
				green = (indexTable[index] >> 8) & 0xFF;
				blue = indexTable[index] & 0xFF;
				break;
			case QOIDecoder.QOI_OP_DIFF:
				red += ((chunk >> 4) & 0b11) - 2;
				green += ((chunk >> 2) & 0b11) - 2;
				blue += (chunk & 0b11) - 2;
				break;
			case QOIDecoder.QOI_OP_LUMA:
				int next = stream.read() & 0xFF;
				int differenceGreen = (chunk & 0b111111) - 32;
				red += differenceGreen - 8 + ((next >> 4) & 0b1111);
				green += differenceGreen;
				blue += differenceGreen - 8 + (next & 0b1111);
				break;
			case QOIDecoder.QOI_OP_RUN:
				for(int z = 0; z < (chunk & 0b111111); z++) {
					pixels[i++] = ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF);
				}
			}

			pixels[i] = indexTable[((red & 0xFF) * 3 + (green & 0xFF) * 5 + (blue & 0xFF) * 7 + 0xFF * 11) & 0b111111] = ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF);
		}

		image.getRaster().setDataElements(0, 0, this.width, this.height, pixels);
		return image;
	}
}
