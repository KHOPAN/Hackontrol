package com.khopan.hackontrol.installer.resources;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

public class Resources {
	public static void paste(String source, File destination) {
		try {
			InputStream inputStream = Resources.class.getResourceAsStream(source);
			byte[] data = inputStream.readAllBytes();
			inputStream.close();
			FileOutputStream outputStream = new FileOutputStream(destination);
			outputStream.write(data);
			outputStream.close();
		} catch(Throwable ignored) {

		}
	}
}
