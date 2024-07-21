package com.khopan.hackontrol.remote.component;

import java.awt.Color;
import java.awt.Component;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.image.BufferedImage;
import java.awt.image.WritableRaster;
import java.io.ByteArrayInputStream;
import java.util.Arrays;

import com.khopan.hackontrol.remote.network.Packet;
import com.khopan.hackontrol.remote.network.PacketProcessor;

public class StreamView extends Component implements PacketProcessor {
	private static final long serialVersionUID = 2380631139944740419L;

	private static final int QOI_OP_RGB   = 0b11111110;
	private static final int QOI_OP_INDEX = 0b00000000;
	private static final int QOI_OP_DIFF  = 0b01000000;
	private static final int QOI_OP_LUMA  = 0b10000000;
	private static final int QOI_OP_RUN   = 0b11000000;
	private static final int OP_MASK      = 0b11000000;

	private final int[] indexTable;

	private int width;
	private int height;
	private Image image;
	private int x;
	private int y;
	private int sourceWidth;
	private int sourceHeight;
	private int[] receiveBuffer;
	private BufferedImage sourceImage;
	private WritableRaster raster;

	public StreamView() {
		this.indexTable = new int[64];
	}

	@SuppressWarnings("deprecation")
	@Override
	public void reshape(int x, int y, int width, int height) {
		super.reshape(x, y, width, height);
		this.width = width;
		this.height = height;
		this.updateImage();
	}

	@Override
	public void paint(Graphics Graphics) {
		Graphics.setColor(new Color(0x000000));
		Graphics.fillRect(0, 0, this.width, this.height);

		if(this.image != null) {
			Graphics.drawImage(this.image, this.x, this.y, null);
		}
	}

	@Override
	public boolean processPacket(Packet packet) {
		ByteArrayInputStream stream = new ByteArrayInputStream(packet.getData());

		switch(packet.getType()) {
		case Packet.PACKET_TYPE_SCREEN_INFORMATION:
			this.sourceWidth = ((stream.read() & 0xFF) << 24) | ((stream.read() & 0xFF) << 16) | ((stream.read() & 0xFF) << 8) | (stream.read() & 0xFF);
			this.sourceHeight = ((stream.read() & 0xFF) << 24) | ((stream.read() & 0xFF) << 16) | ((stream.read() & 0xFF) << 8) | (stream.read() & 0xFF);
			this.receiveBuffer = new int[this.sourceWidth * this.sourceHeight];
			this.sourceImage = new BufferedImage(this.sourceWidth, this.sourceHeight, BufferedImage.TYPE_INT_RGB);
			this.raster = this.sourceImage.getRaster();
			return true;
		case Packet.PACKET_TYPE_STREAM_FRAME:
			int x = ((stream.read() & 0xFF) << 24) | ((stream.read() & 0xFF) << 16) | ((stream.read() & 0xFF) << 8) | (stream.read() & 0xFF);
			int y = ((stream.read() & 0xFF) << 24) | ((stream.read() & 0xFF) << 16) | ((stream.read() & 0xFF) << 8) | (stream.read() & 0xFF);
			int width = ((stream.read() & 0xFF) << 24) | ((stream.read() & 0xFF) << 16) | ((stream.read() & 0xFF) << 8) | (stream.read() & 0xFF);
			int height = ((stream.read() & 0xFF) << 24) | ((stream.read() & 0xFF) << 16) | ((stream.read() & 0xFF) << 8) | (stream.read() & 0xFF);
			Arrays.fill(this.indexTable, 0);
			int red = 0;
			int green = 0;
			int blue = 0;

			for(int i = 0; i < width * height; i++) {
				int chunk = stream.read() & 0xFF;

				if(chunk == StreamView.QOI_OP_RGB) {
					red = stream.read();
					green = stream.read();
					blue = stream.read();
					this.receiveBuffer[i] = this.indexTable[((red & 0xFF) * 3 + (green & 0xFF) * 5 + (blue & 0xFF) * 7 + 0xFF * 11) & 0b111111] = ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF);
					continue;
				}

				switch(chunk & StreamView.OP_MASK) {
				case StreamView.QOI_OP_INDEX:
					int index = chunk & 0b111111;
					red = (this.indexTable[index] >> 16) & 0xFF;
					green = (this.indexTable[index] >> 8) & 0xFF;
					blue = this.indexTable[index] & 0xFF;
					break;
				case StreamView.QOI_OP_DIFF:
					red += ((chunk >> 4) & 0b11) - 2;
					green += ((chunk >> 2) & 0b11) - 2;
					blue += (chunk & 0b11) - 2;
					break;
				case StreamView.QOI_OP_LUMA:
					int next = stream.read() & 0xFF;
					int differenceGreen = (chunk & 0b111111) - 32;
					red += differenceGreen - 8 + ((next >> 4) & 0b1111);
					green += differenceGreen;
					blue += differenceGreen - 8 + (next & 0b1111);
					break;
				case StreamView.QOI_OP_RUN:
					for(int z = 0; z < (chunk & 0b111111); z++) {
						this.receiveBuffer[i++] = ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF);
					}
				}

				this.receiveBuffer[i] = this.indexTable[((red & 0xFF) * 3 + (green & 0xFF) * 5 + (blue & 0xFF) * 7 + 0xFF * 11) & 0b111111] = ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF);
			}

			this.raster.setDataElements(x, y, width, height, this.receiveBuffer);
			this.updateImage();
			return true;
		}

		return false;
	}

	private void updateImage() {
		if(this.sourceImage == null) {
			return;
		}

		int imageWidth = this.sourceImage.getWidth();
		int imageHeight = this.sourceImage.getHeight();
		int newWidth = (int) Math.round(((double) imageWidth) / ((double) imageHeight) * ((double) this.height));
		int newHeight = (int) Math.round(((double) imageHeight) / ((double) imageWidth) * ((double) this.width));

		if(newWidth < this.width) {
			newHeight = this.height;
			this.x = (int) Math.round((((double) this.width) - ((double) newWidth)) * 0.5d);
			this.y = 0;
		} else {
			newWidth = this.width;
			this.x = 0;
			this.y = (int) Math.round((((double) this.height) - ((double) newHeight)) * 0.5d);
		}

		this.image = this.sourceImage.getScaledInstance(newWidth, newHeight, Image.SCALE_DEFAULT);
		this.repaint();
	}
}
