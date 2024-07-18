package com.khopan.hrsp;

import java.awt.image.BufferedImage;

public class QOIDecoder {
	public QOIDecoder() {

	}

	public BufferedImage decode(int width, int height, byte[] data) {
		BufferedImage image = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);
		return image;
	}
}
