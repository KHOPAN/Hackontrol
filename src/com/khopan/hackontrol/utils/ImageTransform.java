package com.khopan.hackontrol.utils;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.image.BufferedImage;

public class ImageTransform {
	private ImageTransform() {}

	public static BufferedImage transform(Image image, int width, int height) {
		BufferedImage result = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
		Graphics2D Graphics = result.createGraphics();

		if(image == null) {
			Graphics.setColor(new Color(0x000000));
			Graphics.fillRect(0, 0, width, height);
			Graphics.dispose();
			return result;
		}

		int imageWidth = image.getWidth(null);
		int imageHeight = image.getHeight(null);
		int x;
		int y;
		int newWidth = (int) Math.round(((double) imageWidth) / ((double) imageHeight) * ((double) height));
		int newHeight = (int) Math.round(((double) imageHeight) / ((double) imageWidth) * ((double) width));

		if(newWidth < width) {
			newHeight = height;
			x = (int) Math.round((((double) width) - ((double) newWidth)) * 0.5d);
			y = 0;
		} else {
			newWidth = width;
			x = 0;
			y = (int) Math.round((((double) height) - ((double) newHeight)) * 0.5d);
		}

		Image scaledImage = image.getScaledInstance(newWidth, newHeight, Image.SCALE_SMOOTH);
		Graphics.drawImage(scaledImage, x, y, null);

		if(newWidth == width && newHeight == height) {
			Graphics.dispose();
			return result;
		}

		if(newHeight == height) {
			BufferedImage side = new BufferedImage(x, height, BufferedImage.TYPE_INT_ARGB);
			Graphics2D sideGraphics = side.createGraphics();

			for(int i = x - newWidth; i > -newWidth; i -= newWidth) {
				sideGraphics.drawImage(scaledImage, i, 0, null);
			}

			sideGraphics.setColor(new Color(0, 0, 0, 200));
			sideGraphics.fillRect(0, 0, x, height);
			sideGraphics.dispose();
			BlurFilter filter = new BlurFilter(25.0f);
			filter.filter(side, side);
			Graphics.drawImage(side, 0, 0, null);
			int rightX = x + newWidth;
			int rightWidth = width - rightX;
			side = new BufferedImage(rightWidth, height, BufferedImage.TYPE_INT_ARGB);
			sideGraphics = side.createGraphics();

			for(int i = 0; i < rightWidth; i += newWidth) {
				sideGraphics.drawImage(scaledImage, i, 0, null);
			}

			sideGraphics.setColor(new Color(0, 0, 0, 200));
			sideGraphics.fillRect(0, 0, rightWidth, height);
			sideGraphics.dispose();
			filter.filter(side, side);
			Graphics.drawImage(side, rightX, 0, null);
		} else {
			BufferedImage topBottom = new BufferedImage(width, y, BufferedImage.TYPE_INT_ARGB);
			Graphics2D topBottomGraphics = topBottom.createGraphics();

			for(int i = y - newHeight; i > -newHeight; i -= newHeight) {
				topBottomGraphics.drawImage(scaledImage, 0, i, null);
			}

			topBottomGraphics.setColor(new Color(0, 0, 0, 200));
			topBottomGraphics.fillRect(0, 0, width, y);
			topBottomGraphics.dispose();
			BlurFilter filter = new BlurFilter(25.0f);
			filter.filter(topBottom, topBottom);
			Graphics.drawImage(topBottom, 0, 0, null);
			int bottomY = y + newHeight;
			int bottomHeight = height - bottomY;
			topBottom = new BufferedImage(width, bottomHeight, BufferedImage.TYPE_INT_ARGB);
			topBottomGraphics = topBottom.createGraphics();

			for(int i = 0; i < bottomHeight; i += newHeight) {
				topBottomGraphics.drawImage(scaledImage, 0, i, null);
			}

			topBottomGraphics.setColor(new Color(0, 0, 0, 200));
			topBottomGraphics.fillRect(0, 0, width, bottomHeight);
			topBottomGraphics.dispose();
			filter.filter(topBottom, topBottom);
			Graphics.drawImage(topBottom, 0, bottomY, null);
		}

		Graphics.dispose();
		return result;
	}
}
