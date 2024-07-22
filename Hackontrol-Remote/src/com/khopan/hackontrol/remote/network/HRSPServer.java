package com.khopan.hackontrol.remote.network;

import java.io.ByteArrayInputStream;
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

	public static void start(DefaultListModel<RemoteSession> model, Runnable onClose) {
		try {
			HackontrolRemote.LOGGER.info("Wait for incoming connection...");
			ServerSocket server = new ServerSocket(42485);

			try {
				while(true) {
					Socket socket = server.accept();
					HackontrolRemote.LOGGER.info("Client connected: {}", socket.getInetAddress().getHostAddress());
					Thread thread = new Thread(() -> HRSPServer.handleConnection(model, onClose, socket));
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

	private static void handleConnection(DefaultListModel<RemoteSession> model, Runnable onClose, Socket socket) {
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
			Packet packet = Packet.readPacket(inputStream);

			if(packet.getType() != Packet.PACKET_TYPE_INFORMATION) {
				throw new IllegalArgumentException("Invalid packet type, the first packet sent must be PACKET_TYPE_INFORMATION");
			}

			ByteArrayInputStream informationStream = new ByteArrayInputStream(packet.getData());
			int width = ((informationStream.read() & 0xFF) << 24) | ((informationStream.read() & 0xFF) << 16) | ((informationStream.read() & 0xFF) << 8) | (informationStream.read() & 0xFF);
			int height = ((informationStream.read() & 0xFF) << 24) | ((informationStream.read() & 0xFF) << 16) | ((informationStream.read() & 0xFF) << 8) | (informationStream.read() & 0xFF);
			String username = new String(informationStream.readAllBytes(), StandardCharsets.UTF_8);
			session = new RemoteSession(socket, inputStream, outputStream, model, onClose, width, height, username);
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
