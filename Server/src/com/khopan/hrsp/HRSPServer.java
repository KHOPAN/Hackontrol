package com.khopan.hrsp;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.charset.StandardCharsets;

import javax.imageio.ImageIO;
import javax.swing.JFrame;
import javax.swing.WindowConstants;

public class HRSPServer {
	public static void main(String[] args) throws Throwable {
		JFrame frame = new JFrame();
		frame.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
		frame.setLayout(new BorderLayout());
		ImageView view = new ImageView();
		frame.add(view, BorderLayout.CENTER);
		frame.setSize(600, 400);
		frame.setLocationRelativeTo(null);
		frame.setVisible(true);
		System.out.println("Wait for incoming connection...");
		ServerSocket server = new ServerSocket(42485);
		Socket socket = server.accept();
		System.out.println("Client connected: " + socket.getInetAddress().getHostName());
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
			byte[] bytes = inputStream.readNBytes(4);
			int size = ((bytes[0] & 0xFF) << 24) | ((bytes[1] & 0xFF) << 16) | ((bytes[2] & 0xFF) << 8) | (bytes[3] & 0xFF);
			byte[] data = inputStream.readNBytes(size);
			BufferedImage image = ImageIO.read(new ByteArrayInputStream(data));
			view.setImage(image);
		}
	}

	private static class ImageView extends Component {
		private static final long serialVersionUID = -878340789288529563L;

		private BufferedImage image;

		private void setImage(BufferedImage image) {
			this.image = image;
			this.repaint();
		}

		@Override
		public void paint(Graphics Graphics) {
			Graphics2D Graphics2D = (Graphics2D) Graphics;
			Graphics2D.drawImage(this.image, 0, 0, null);
		}
	}
}
