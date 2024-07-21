package com.khopan.hackontrol.remote;

import java.awt.BorderLayout;

import javax.swing.JFrame;
import javax.swing.WindowConstants;

public class HackontrolRemote {
	private static HackontrolRemote INSTANCE;

	private HackontrolRemote() {
		HackontrolRemote.INSTANCE = this;
		JFrame frame = new JFrame();
		frame.setTitle("Hackontrol Remote");
		frame.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
		frame.setLayout(new BorderLayout());
		frame.add(null, BorderLayout.CENTER);
		frame.setSize(600, 400);
		frame.setLocationRelativeTo(null);
		frame.setAlwaysOnTop(true);
		frame.setVisible(true);
	}

	public static void main(String[] args) {
		HackontrolRemote.getInstance();
		/*System.out.println("Wait for incoming connection...");
		ServerSocket server = new ServerSocket(42485);
		Socket socket = server.accept();
		System.out.println("Client connected: " + socket.getInetAddress().getHostAddress());
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
		QOIDecoder decoder = new QOIDecoder();

		while(true) {
			Packet packet = Packet.readPacket(inputStream);

			if(decoder.packet(packet)) {
				view.setImage(decoder.getImage());
			}
		}*/
	}

	public static HackontrolRemote getInstance() {
		if(HackontrolRemote.INSTANCE == null) {
			HackontrolRemote.INSTANCE = new HackontrolRemote();
		}

		return HackontrolRemote.INSTANCE;
	}
}
