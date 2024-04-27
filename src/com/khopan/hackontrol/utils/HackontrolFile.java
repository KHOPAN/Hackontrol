package com.khopan.hackontrol.utils;

import java.io.File;
import java.text.CharacterIterator;
import java.text.StringCharacterIterator;

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

	public static FileCountAndSize getFileCountAndSize(File root) {
		if(root == null) {
			return new FileCountAndSize(0L, 0L);
		}

		if(root.isDirectory()) {
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

		return new FileCountAndSize(1L, root.length());
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
