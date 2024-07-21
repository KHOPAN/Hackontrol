package com.khopan.hackontrol.remote.network;

import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.nio.charset.StandardCharsets;

import javax.swing.DefaultListModel;

import com.khopan.hackontrol.remote.HackontrolRemote;
import com.khopan.hackontrol.remote.session.RemoteSession;

public class HRSPServer {
	private HRSPServer() {}

	private static int Count;

	public static void start(DefaultListModel<RemoteSession> model) {
		try {
			HackontrolRemote.LOGGER.info("Wait for incoming connection...");
			ServerSocket server = new ServerSocket(42485);

			try {
				while(true) {
					Socket socket = server.accept();
					HackontrolRemote.LOGGER.info("Client connected: {}", socket.getInetAddress().getHostAddress());
					Thread thread = new Thread(() -> HRSPServer.handleConnection(model, socket));
					thread.setName(HackontrolRemote.NAME + " Connection Handler #" + (HRSPServer.Count++));
					thread.start();
				}
			} finally {
				server.close();
			}
		} catch(Throwable Errors) {
			HackontrolRemote.LOGGER.error("HRSP Server Error", Errors);
		}
	}

	private static void handleConnection(DefaultListModel<RemoteSession> model, Socket socket) {
		RemoteSession session = null;

		try {
			InputStream inputStream = socket.getInputStream();
			String response = new String(inputStream.readNBytes(16), StandardCharsets.UTF_8);

			if(!"HRSP 1.0 CONNECT".equals(response)) {
				socket.close();
				return;
			}

			OutputStream outputStream = socket.getOutputStream();
			outputStream.write("HRSP 1.0 OK".getBytes(StandardCharsets.UTF_8));
			outputStream.flush();
			session = new RemoteSession(socket, inputStream, outputStream);
			model.addElement(session);
			session.start();
		} catch(SocketException ignored) {
		} catch(Throwable Errors) {
			HackontrolRemote.LOGGER.error("HRSP Server Error", Errors);
		}

		model.removeElement(session);
		HRSPServer.Count--;
	}
}
