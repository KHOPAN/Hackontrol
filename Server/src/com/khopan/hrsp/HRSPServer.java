package com.khopan.hrsp;

import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.charset.StandardCharsets;

public class HRSPServer {
	public static void main(String[] args) throws Throwable {
		ServerSocket server = new ServerSocket(42485);
		Socket socket = server.accept();
		InputStream inputStream = socket.getInputStream();
		byte[] data = inputStream.readNBytes(16);

		if(!"HRSP 1.0 CONNECT".equals(new String(data, StandardCharsets.UTF_8))) {
			socket.close();
			server.close();
			return;
		}

		OutputStream outputStream = socket.getOutputStream();
		outputStream.write("HRSP 1.0 OK".getBytes(StandardCharsets.UTF_8));
		server.close();
	}
}
