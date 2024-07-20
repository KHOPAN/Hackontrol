package com.khopan.hrsp;

import java.awt.image.BufferedImage;
import java.awt.image.WritableRaster;
import java.io.ByteArrayInputStream;

public class QOIDecoder {
	public static final int QOI_OP_RGB   = 0b11111110;
	public static final int QOI_OP_INDEX = 0b00000000;
	public static final int QOI_OP_DIFF  = 0b01000000;
	public static final int QOI_OP_LUMA  = 0b10000000;
	public static final int QOI_OP_RUN   = 0b11000000;
	public static final int OP_MASK      = 0b11000000;

	private final int[] indexTable;

	private int width;
	private int height;
	private BufferedImage image;
	private int[] pixels;
	private WritableRaster raster;

	public QOIDecoder() {
		this.indexTable = new int[64];
	}

	public boolean packet(Packet packet) {
		if(packet == null) {
			return false;
		}

		byte[] data = packet.getData();

		switch(packet.getType()) {
		case Packet.PACKET_TYPE_SCREEN_INFORMATION:
			this.width = ((data[0] & 0xFF) << 24) | ((data[1] & 0xFF) << 16) | ((data[2] & 0xFF) << 8) | (data[3] & 0xFF);
			this.height = ((data[4] & 0xFF) << 24) | ((data[5] & 0xFF) << 16) | ((data[6] & 0xFF) << 8) | (data[7] & 0xFF);
			this.image = new BufferedImage(this.width, this.height, BufferedImage.TYPE_INT_RGB);
			this.pixels = new int[this.width * this.height];
			this.raster = this.image.getRaster();
			break;
		case Packet.PACKET_TYPE_STREAM_FRAME:
			this.decode(data);
			return true;
		}

		return false;
	}

	public BufferedImage getImage() {
		return this.image;
	}

	private void decode(byte[] data) {
		ByteArrayInputStream stream = new ByteArrayInputStream(data);
		int x = ((stream.read() & 0xFF) << 24) | ((stream.read() & 0xFF) << 16) | ((stream.read() & 0xFF) << 8) | (stream.read() & 0xFF);
		int y = ((stream.read() & 0xFF) << 24) | ((stream.read() & 0xFF) << 16) | ((stream.read() & 0xFF) << 8) | (stream.read() & 0xFF);
		int width = ((stream.read() & 0xFF) << 24) | ((stream.read() & 0xFF) << 16) | ((stream.read() & 0xFF) << 8) | (stream.read() & 0xFF);
		int height = ((stream.read() & 0xFF) << 24) | ((stream.read() & 0xFF) << 16) | ((stream.read() & 0xFF) << 8) | (stream.read() & 0xFF);
		int red = 0;
		int green = 0;
		int blue = 0;

		for(int i = 0; i < width * height; i++) {
			int chunk = stream.read() & 0xFF;

			if(chunk == QOIDecoder.QOI_OP_RGB) {
				red = stream.read();
				green = stream.read();
				blue = stream.read();
				this.pixels[i] = this.indexTable[((red & 0xFF) * 3 + (green & 0xFF) * 5 + (blue & 0xFF) * 7 + 0xFF * 11) & 0b111111] = ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF);
				continue;
			}

			switch(chunk & QOIDecoder.OP_MASK) {
			case QOIDecoder.QOI_OP_INDEX:
				int index = chunk & 0b111111;
				red = (this.indexTable[index] >> 16) & 0xFF;
				green = (this.indexTable[index] >> 8) & 0xFF;
				blue = this.indexTable[index] & 0xFF;
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
					this.pixels[i++] = ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF);
				}
			}

			this.pixels[i] = this.indexTable[((red & 0xFF) * 3 + (green & 0xFF) * 5 + (blue & 0xFF) * 7 + 0xFF * 11) & 0b111111] = ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF);
		}

		this.raster.setDataElements(x, y, width, height, this.pixels);
	}
}
