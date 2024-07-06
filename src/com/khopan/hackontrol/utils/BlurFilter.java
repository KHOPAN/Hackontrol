package com.khopan.hackontrol.utils;

import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.awt.image.BufferedImageOp;
import java.awt.image.ColorModel;
import java.awt.image.Kernel;

public class BlurFilter implements BufferedImageOp {
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
