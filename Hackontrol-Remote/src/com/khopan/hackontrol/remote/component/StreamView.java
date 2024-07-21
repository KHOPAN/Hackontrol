package com.khopan.hackontrol.remote.component;

import java.awt.Color;
import java.awt.Component;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;
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
	private BufferedImage sourceImage;
	private int[] receiveBuffer;

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
			this.sourceImage = new BufferedImage(this.sourceWidth, this.sourceHeight, BufferedImage.TYPE_INT_RGB);
			this.receiveBuffer = ((DataBufferInt) this.sourceImage.getRaster().getDataBuffer()).getData();
			return true;
		case Packet.PACKET_TYPE_STREAM_FRAME:
			this.decodeImage(stream);
			return true;
		}

		return false;
	}

	private void decodeImage(ByteArrayInputStream stream) {
		int startX = ((stream.read() & 0xFF) << 24) | ((stream.read() & 0xFF) << 16) | ((stream.read() & 0xFF) << 8) | (stream.read() & 0xFF);
		int startY = ((stream.read() & 0xFF) << 24) | ((stream.read() & 0xFF) << 16) | ((stream.read() & 0xFF) << 8) | (stream.read() & 0xFF);
		int width = ((stream.read() & 0xFF) << 24) | ((stream.read() & 0xFF) << 16) | ((stream.read() & 0xFF) << 8) | (stream.read() & 0xFF);
		int height = ((stream.read() & 0xFF) << 24) | ((stream.read() & 0xFF) << 16) | ((stream.read() & 0xFF) << 8) | (stream.read() & 0xFF);
		Arrays.fill(this.indexTable, 0);
		int red = 0;
		int green = 0;
		int blue = 0;
		int run = 0;

		for(int y = startY; y < startY + height; y++) {
			for(int x = startX; x < startX + width; x++) {
				int pixelIndex = y * this.sourceWidth + x;

				if(run > 0) {
					this.subtract(pixelIndex, ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF));
					run--;
					continue;
				}

				int chunk = stream.read() & 0xFF;

				if(chunk == StreamView.QOI_OP_RGB) {
					red = stream.read();
					green = stream.read();
					blue = stream.read();
					this.subtract(pixelIndex, this.indexTable[((red & 0xFF) * 3 + (green & 0xFF) * 5 + (blue & 0xFF) * 7 + 0xFF * 11) & 0b111111] = ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF));
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
					this.subtract(pixelIndex, ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF));
					run = (chunk & 0b111111);
					continue;
				}

				this.subtract(pixelIndex, this.indexTable[((red & 0xFF) * 3 + (green & 0xFF) * 5 + (blue & 0xFF) * 7 + 0xFF * 11) & 0b111111] = ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF));
			}
		}

		this.updateImage();
	}

	private void subtract(int index, int color) {
		int red = ((this.receiveBuffer[index] >> 16) & 0xFF) - ((color >> 16) & 0xFF);
		int green = ((this.receiveBuffer[index] >> 8) & 0xFF) - ((color >> 8) & 0xFF);
		int blue = (this.receiveBuffer[index] & 0xFF) - (color & 0xFF);
		this.receiveBuffer[index] = ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF);
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
