package com.khopan.hackontrol.utils;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.awt.image.BufferedImageOp;
import java.awt.image.ColorModel;
import java.awt.image.Kernel;

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

		BufferedImageOp filter = new BlurFilter(25.0f);

		if(newHeight == height) {
			if(x > 0) {
				BufferedImage left = new BufferedImage(x, height, BufferedImage.TYPE_INT_ARGB);
				Graphics2D leftGraphics = left.createGraphics();

				for(int i = x - newWidth; i > -newWidth; i -= newWidth) {
					leftGraphics.drawImage(scaledImage, i, 0, null);
				}

				leftGraphics.setColor(new Color(0, 0, 0, 200));
				leftGraphics.fillRect(0, 0, x, height);
				leftGraphics.dispose();
				filter.filter(left, left);
				Graphics.drawImage(left, 0, 0, null);
			}

			int rightX = x + newWidth;
			int rightWidth = width - rightX;

			if(rightWidth > 0) {
				BufferedImage right = new BufferedImage(rightWidth, height, BufferedImage.TYPE_INT_ARGB);
				Graphics2D rightGraphics = right.createGraphics();

				for(int i = 0; i < rightWidth; i += newWidth) {
					rightGraphics.drawImage(scaledImage, i, 0, null);
				}

				rightGraphics.setColor(new Color(0, 0, 0, 200));
				rightGraphics.fillRect(0, 0, rightWidth, height);
				rightGraphics.dispose();
				filter.filter(right, right);
				Graphics.drawImage(right, rightX, 0, null);
			}
		} else {
			if(y > 0) {
				BufferedImage top = new BufferedImage(width, y, BufferedImage.TYPE_INT_ARGB);
				Graphics2D topGraphics = top.createGraphics();

				for(int i = y - newHeight; i > -newHeight; i -= newHeight) {
					topGraphics.drawImage(scaledImage, 0, i, null);
				}

				topGraphics.setColor(new Color(0, 0, 0, 200));
				topGraphics.fillRect(0, 0, width, y);
				topGraphics.dispose();
				filter.filter(top, top);
				Graphics.drawImage(top, 0, 0, null);
			}

			int bottomY = y + newHeight;
			int bottomHeight = height - bottomY;

			if(bottomHeight > 0) {
				BufferedImage bottom = new BufferedImage(width, bottomHeight, BufferedImage.TYPE_INT_ARGB);
				Graphics2D bottomGraphics = bottom.createGraphics();

				for(int i = 0; i < bottomHeight; i += newHeight) {
					bottomGraphics.drawImage(scaledImage, 0, i, null);
				}

				bottomGraphics.setColor(new Color(0, 0, 0, 200));
				bottomGraphics.fillRect(0, 0, width, bottomHeight);
				bottomGraphics.dispose();
				filter.filter(bottom, bottom);
				Graphics.drawImage(bottom, 0, bottomY, null);
			}
		}

		Graphics.dispose();
		return result;
	}

	private static class BlurFilter implements BufferedImageOp {
		private final float radius;
		private final Kernel kernel;

		public BlurFilter(float radius) {
			this.radius = radius;
			int radiusInteger = (int) Math.ceil(this.radius);
			float radiusSquared = this.radius * this.radius;
			int rows = radiusInteger * 2 + 1;
			float[] matrix = new float[rows];
			float sigma = radius / 3.0f;
			float sigmaSquared = sigma * sigma * 2.0f;
			float squareRoot = (float) Math.sqrt(Math.PI * sigma * 2.0f);
			float total = 0.0f;
			int index = 0;

			for(int i = -radiusInteger; i <= radiusInteger; i++) {
				float distance = i * i;

				if(distance > radiusSquared) {
					matrix[index] = 0;
				} else {
					matrix[index] = (float) Math.exp(-distance / sigmaSquared) / squareRoot;
				}

				total += matrix[index];
				index++;
			}

			for(int i = 0; i < rows; i++) {
				matrix[i] /= total;
			}

			this.kernel = new Kernel(rows, 1, matrix);
		}

		@Override
		public BufferedImage filter(BufferedImage source, BufferedImage destination) {
			int width = source.getWidth();
			int height = source.getHeight();

			if(destination == null) {
				destination = this.createCompatibleDestImage(source, null);
			}

			int[] input = new int[width * height];
			source.getRGB(0, 0, width, height, input, 0, width);
			int[] output = new int[width * height];
			this.convolve(input, output, width, height);
			this.convolve(output, input, height, width);
			destination.setRGB(0, 0, width, height, input, 0, width);
			return destination;
		}

		private void convolve(int[] input, int[] output, int width, int height) {
			float[] matrix = this.kernel.getKernelData(null);
			int kernelHalfWidth = this.kernel.getWidth() / 2;

			for(int y = 0; y < height; y++) {
				int index = y;
				int offset = y * width;

				for(int x = 0; x < width; x++) {
					output[index] = this.calculateOutput(kernelHalfWidth, matrix, x, width, input, offset);
					index += height;
				}
			}
		}

		private int calculateOutput(int kernelHalfWidth, float[] matrix, int x, int width, int[] input, int offset) {
			float alpha = 0.0f;
			float red = 0.0f;
			float green = 0.0f;
			float blue = 0.0f;

			for(int i = -kernelHalfWidth; i <= kernelHalfWidth; i++) {
				float value = matrix[kernelHalfWidth + i];

				if(value == 0.0f) {
					continue;
				}

				int offsetX = x + i;

				if(offsetX < 0 || offsetX >= width) {
					offsetX = (x + width) % width;
				}

				int color = input[offset + offsetX];
				alpha += ((color >> 24) & 0xFF) * value;
				red += ((color >> 16) & 0xFF) * value;
				green += ((color >> 8) & 0xFF) * value;
				blue += (color & 0xFF) * value;
			}

			return (this.limit(Math.round(alpha + 0.5f)) << 24) | (this.limit(Math.round(red + 0.5f)) << 16) | (this.limit(Math.round(green + 0.5f)) << 8) | this.limit(Math.round(blue + 0.5f));
		}

		private int limit(int value) {
			return value < 0 ? 0 : value > 255 ? 255 : value;
		}

		@Override
		public Rectangle2D getBounds2D(BufferedImage source) {
			return new Rectangle(0, 0, source.getWidth(), source.getHeight());
		}

		@Override
		public BufferedImage createCompatibleDestImage(BufferedImage source, ColorModel destination) {
			if(destination == null) {
				destination = source.getColorModel();
			}

			return new BufferedImage(destination, destination.createCompatibleWritableRaster(source.getWidth(), source.getHeight()), destination.isAlphaPremultiplied(), null);
		}

		@Override
		public Point2D getPoint2D(Point2D source, Point2D destination) {
			if(destination == null) {
				destination = new Point2D.Double();
			}

			destination.setLocation(source.getX(), source.getY());
			return destination;
		}

		@Override
		public RenderingHints getRenderingHints() {
			return null;
		}
	}
}
