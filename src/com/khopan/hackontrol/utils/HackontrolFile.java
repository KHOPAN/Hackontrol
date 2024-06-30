package com.khopan.hackontrol.utils;

import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.image.BufferedImage;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.text.CharacterIterator;
import java.text.StringCharacterIterator;
import java.util.ArrayList;
import java.util.List;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

import javax.imageio.ImageIO;
import javax.swing.Icon;
import javax.swing.JPanel;
import javax.swing.filechooser.FileSystemView;

import net.dv8tion.jda.api.utils.FileUpload;

public class HackontrolFile {
	private HackontrolFile() {}

	public static String getFileSizeDisplay(long bytes) {
		if(-1000 < bytes && bytes < 1000) {
			return bytes + " B";
		}

		CharacterIterator iterator = new StringCharacterIterator("kMGTPE");

		while(bytes <= -999950 || bytes >= 999950) {
			bytes /= 1000;
			iterator.next();
		}

		return String.format("%.1f %cB", ((double) bytes) / 1000.0d, iterator.current());
	}

	public static boolean delete(File root) {
		if(root == null) {
			return false;
		}

		if(!root.isDirectory()) {
			return root.delete();
		}

		File[] list = root.listFiles();

		if(list == null || list.length == 0) {
			return root.delete();
		}

		boolean success = true;

		for(int i = 0; i < list.length; i++) {
			if(!HackontrolFile.delete(list[i])) {
				success = false;
			}
		}

		return success && root.delete();
	}

	public static FileUpload upload(File input) {
		if(input == null) {
			throw new NullPointerException("Input cannot be null");
		}

		String name = input.getName();

		if(input.isFile()) {
			return FileUpload.fromData(input, name);
		}

		try {
			List<File> list = new ArrayList<>();
			HackontrolFile.retrieveFileList(list, input);
			ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
			ZipOutputStream zipOutputStream = new ZipOutputStream(byteArrayOutputStream);
			int startIndex = input.getAbsolutePath().length() + 1;

			for(int i = 0; i < list.size(); i++) {
				File file = list.get(i);
				String fileName = file.getAbsolutePath();
				ZipEntry entry = new ZipEntry(fileName.substring(startIndex, fileName.length()));
				zipOutputStream.putNextEntry(entry);
				FileInputStream fileInputStream = new FileInputStream(file);
				byte[] data = fileInputStream.readAllBytes();
				fileInputStream.close();
				zipOutputStream.write(data);
			}

			zipOutputStream.close();
			return FileUpload.fromData(byteArrayOutputStream.toByteArray(), name + ".zip");
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

		BufferedImage image = new BufferedImage(icon.getIconWidth(), icon.getIconHeight(), BufferedImage.TYPE_INT_ARGB);
		Graphics2D Graphics = image.createGraphics();
		Graphics.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		icon.paintIcon(new JPanel(), Graphics, 0, 0);
		Graphics.dispose();
		return image;
	}

	private static void retrieveFileList(List<File> list, File root) {
		if(root == null) {
			return;
		}

		if(!root.isDirectory()) {
			list.add(root);
			return;
		}

		File[] fileList = root.listFiles();

		if(fileList == null) {
			return;
		}

		for(int i = 0; i < fileList.length; i++) {
			HackontrolFile.retrieveFileList(list, fileList[i]);
		}
	}

	public static FileCountAndSize getFileCountAndSize(File root) {
		if(root == null) {
			return new FileCountAndSize(0L, 0L);
		}

		if(!root.isDirectory()) {
			return new FileCountAndSize(1L, root.length());
		}

		File[] list = root.listFiles();

		if(list == null) {
			return new FileCountAndSize(0L, 0L);
		}

		long count = 0L;
		long size = 0L;

		for(int i = 0; i < list.length; i++) {
			FileCountAndSize countAndSize = HackontrolFile.getFileCountAndSize(list[i]);
			count += countAndSize.fileCount;
			size += countAndSize.folderSize;
		}

		return new FileCountAndSize(count, size);
	}

	public static class FileCountAndSize {
		public final long fileCount;
		public final long folderSize;

		private FileCountAndSize(long fileCount, long folderSize) {
			this.fileCount = fileCount;
			this.folderSize = folderSize;
		}
	}
}
