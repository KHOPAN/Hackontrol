package com.khopan.hackontrol.utils;

import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.image.BufferedImage;
import java.awt.image.RenderedImage;
import java.io.ByteArrayOutputStream;
import java.io.File;

import javax.imageio.ImageIO;
import javax.swing.Icon;
import javax.swing.JPanel;
import javax.swing.filechooser.FileSystemView;

public class ImageUtils {
	private ImageUtils() {}

	public static BufferedImage iconToBufferedImage(Icon icon) {
		if(icon == null) {
			throw new NullPointerException("Icon cannot be null");
		}

		int width = icon.getIconWidth();
		int height = icon.getIconHeight();
		BufferedImage image = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
		Graphics2D Graphics = image.createGraphics();
		Graphics.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		icon.paintIcon(new JPanel(), Graphics, 0, 0);
		Graphics.dispose();
		return image;
	}

	public static byte[] imageToByteArray(RenderedImage image) {
		if(image == null) {
			throw new NullPointerException("Image cannot be null");
		}

		try {
			ByteArrayOutputStream stream = new ByteArrayOutputStream();
			ImageIO.write(image, "png", stream);
			return stream.toByteArray();
		} catch(Throwable Errors) {
			throw new RuntimeException(Errors);
		}
	}

	public static BufferedImage getFileImage(File file, int width, int height) {
		if(file == null) {
			throw new NullPointerException("File cannot be null");
		}

		try {
			BufferedImage image = ImageIO.read(file);

			if(image == null) {
				throw new RuntimeException();
			}

			int imageWidth = image.getWidth();
			int imageHeight = image.getHeight();
			int newWidth = 0;
			int newHeight = 0;

			if(imageWidth > imageHeight) {
				newWidth = width;
				newHeight = (int) Math.round(((double) imageHeight) / ((double) imageWidth) * ((double) width));
			} else {
				newWidth = (int) Math.round(((double) imageWidth) / ((double) imageHeight) * ((double) height));
				newHeight = height;
			}

			BufferedImage result = new BufferedImage(newWidth, newHeight, BufferedImage.TYPE_INT_ARGB);
			Graphics2D Graphics = result.createGraphics();
			Graphics.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
			Graphics.drawImage(image.getScaledInstance(newWidth, newHeight, BufferedImage.SCALE_SMOOTH), 0, 0, null);
			Graphics.dispose();
			return result;
		} catch(Throwable Errors) {

		}

		FileSystemView view = FileSystemView.getFileSystemView();
		Icon icon = view.getSystemIcon(file, width, height);

		if(icon == null) {
			return null;
		}

		return ImageUtils.iconToBufferedImage(icon);
	}
}
