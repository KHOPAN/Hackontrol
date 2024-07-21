package com.khopan.hackontrol.remote.network;

import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.nio.charset.StandardCharsets;

import com.khopan.hackontrol.remote.HackontrolRemote;

public class HRSPServer {
	private HRSPServer() {}

	public static void start(PacketProcessor processor) {
		try {
			HackontrolRemote.LOGGER.info("Wait for incoming connection...");
			ServerSocket server = new ServerSocket(42485);
			Socket socket = server.accept();
			HackontrolRemote.LOGGER.info("Client connected: {}", socket.getInetAddress().getHostAddress());
			InputStream inputStream = socket.getInputStream();
			String response = new String(inputStream.readNBytes(16), StandardCharsets.UTF_8);

			if(!"HRSP 1.0 CONNECT".equals(response)) {
				socket.close();
				server.close();
				return;
			}

			OutputStream outputStream = socket.getOutputStream();
			outputStream.write("HRSP 1.0 OK".getBytes(StandardCharsets.UTF_8));
			outputStream.flush();

			while(true) {
				try {
					processor.processPacket(Packet.readPacket(inputStream));
				} catch(SocketException Exception) {
					HackontrolRemote.LOGGER.info("Client disconnected, exiting...");
					System.exit(0);
				}
			}
		} catch(Throwable Errors) {
			HackontrolRemote.LOGGER.error("HRSP Server Error", Errors);
		}
	}
}
