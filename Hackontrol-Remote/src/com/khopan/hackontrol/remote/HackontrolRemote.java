package com.khopan.hackontrol.remote;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.image.BufferedImage;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.charset.StandardCharsets;

import javax.swing.JFrame;
import javax.swing.WindowConstants;

public class HackontrolRemote {
	public static void main(String[] args) throws Throwable {
		JFrame frame = new JFrame();
		frame.setTitle("Hackontrol Remote");
		frame.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
		frame.setLayout(new BorderLayout());
		ImageView view = new ImageView();
		frame.add(view, BorderLayout.CENTER);
		frame.setSize(600, 400);
		frame.setLocationRelativeTo(null);
		frame.setAlwaysOnTop(true);
		frame.setVisible(true);
		System.out.println("Wait for incoming connection...");
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
		}
	}

	private static class ImageView extends Component {
		private static final long serialVersionUID = -878340789288529563L;

		private BufferedImage sourceImage;
		private Image image;
		private int x;
		private int y;
		private int width;
		private int height;

		private void setImage(BufferedImage image) {
			this.sourceImage = image;
			this.update();
		}

		@SuppressWarnings("deprecation")
		@Override
		public void reshape(int x, int y, int width, int height) {
			super.reshape(x, y, width, height);
			this.width = width;
			this.height = height;
			this.update();
		}

		private void update() {
			if(this.sourceImage == null) {
				return;
			}

			int imageWidth = this.sourceImage.getWidth();
			int imageHeight = this.sourceImage.getHeight();
			int newWidth = (int) Math.round(((double) imageWidth) / ((double) imageHeight) * ((double) this.height));
			int newHeight = (int) Math.round(((double) imageHeight) / ((double) imageWidth) * ((double) this.width));

			if(newWidth < this.width) {
				newHeight = this.height;
				this.x = (int) Math.round((((double) this.width) - ((double) newWidth)) * 0.5d);
				this.y = 0;
			} else {
				newWidth = this.width;
				this.x = 0;
				this.y = (int) Math.round((((double) this.height) - ((double) newHeight)) * 0.5d);
			}

			this.image = this.sourceImage.getScaledInstance(newWidth, newHeight, Image.SCALE_FAST);
			this.repaint();
		}

		@Override
		public void paint(Graphics Graphics) {
			Graphics.setColor(new Color(0x000000));
			Graphics.fillRect(0, 0, this.width, this.height);

			if(this.image != null) {
				Graphics.drawImage(this.image, this.x, this.y, null);
			}
		}
	}
}
