package com.khopan.hackontrol;

import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;

import javax.imageio.ImageIO;

public class CameraDevice {
	public final String deviceName;
	public final String symbolicLink;

	public CameraDevice(String deviceName, String symbolicLink) {
		this.deviceName = deviceName;
		this.symbolicLink = symbolicLink;
	}

	public String getDeviceName() {
		return this.deviceName;
	}

	public String getSymbolicLink() {
		return this.symbolicLink;
	}

	public BufferedImage capture() {
		try {
			byte[] data = NativeLibrary.capture(this, true);

			if(data == null || data.length == 0) {
				throw new RuntimeException();
			}

			ByteArrayInputStream stream = new ByteArrayInputStream(data);
			stream.readNBytes(8);
			return ImageIO.read(stream);
		} catch(Throwable Errors) {
			if(!(Errors instanceof UnsupportedOperationException)) {
				throw new RuntimeException(Errors);
			}
		}

		byte[] data;

		try {
			data = NativeLibrary.capture(this, false);

			if(data == null || data.length == 0) {
				return null;
			}
		} catch(Throwable Errors) {
			throw new RuntimeException(Errors);
		}

		int width;
		int height;

		try {
			ByteArrayInputStream stream = new ByteArrayInputStream(data);
			width = this.byteToInt(stream.readNBytes(4));
			height = this.byteToInt(stream.readNBytes(4));
			data = stream.readAllBytes();
		} catch(Throwable Errors) {
			throw new RuntimeException(Errors);
		}

		BufferedImage image = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);

		for(int i = 0; i < data.length; i += 4) {
			byte y1 = data[i];
			byte u = data[i + 1];
			byte y2 = data[i + 2];
			byte v = data[i + 3];
			int index = i / 2;
			image.setRGB(index % width, index / width, this.yuvToPackedRGB(y1, u, v));
			index++;
			image.setRGB(index % width, index / width, this.yuvToPackedRGB(y2, u, v));
		}

		return image;
	}

	private int yuvToPackedRGB(byte y, byte u, byte v) {
		double Y = 1.164d * (((double) (y & 0xFF)) - 16.0d);
		double U = (double) (u & 0xFF) - 128.0d;
		double V = (double) (v & 0xFF) - 128.0d;
		int red   = Math.min(Math.max((int) Math.round(Y + 1.596d * V),              0), 255);
		int green = Math.min(Math.max((int) Math.round(Y - 0.813d * V - 0.391d * U), 0), 255);
		int blue  = Math.min(Math.max((int) Math.round(Y + 2.018d * U),              0), 255);
		return ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF);
	}

	private int byteToInt(byte[] data) {
		if(data == null || data.length != 4) {
			return 0;
		}

		return ((data[0] & 0xFF) << 24) | ((data[1] & 0xFF) << 16) | ((data[2] & 0xFF) << 8) | (data[3] & 0xFF);
	}
}
